/*
**
** Copyright 2011, Accenture Ltd
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define GL_GLEXT_PROTOTYPES 1
#include "GLRenderer.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"
#include "GL/glext.h"

#include "OpenGLES1_1Parser.h"
#include "OpenGLES2Parser.h"

#include "gles2_emulator_constants.h"
#include "debug.h"

/* Coordinates for a simple test cube. */
float testCube[][3] = {
		{ -1.0, -1.0, -1.0 },
		{  1.0, -1.0, -1.0 },
		{  1.0,  1.0, -1.0 },
		{ -1.0,  1.0, -1.0 },
		{ -1.0, -1.0,  1.0 },
		{  1.0, -1.0,  1.0 },
		{  1.0,  1.0,  1.0 },
		{ -1.0,  1.0,  1.0 }
};


#define TESTCUBE_ALPHA 0.5

int	totalBufferSize = HostBufferSize;

bool checkFramebufferStatus()
{
    // check FBO status
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        DBG_PRINT("Framebuffer complete.\n");
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        DBG_PRINT("[ERROR] Framebuffer incomplete: Attachment is NOT complete.\n");
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        DBG_PRINT("[ERROR] Framebuffer incomplete: No image is attached to FBO.\n");
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        DBG_PRINT("[ERROR] Framebuffer incomplete: Attached images have different dimensions.\n");
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        DBG_PRINT("[ERROR] Framebuffer incomplete: Color attached images have different internal formats.\n");
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        DBG_PRINT("[ERROR] Framebuffer incomplete: Draw buffer.\n");
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        DBG_PRINT("[ERROR] Framebuffer incomplete: Read buffer.\n");
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        DBG_PRINT("[ERROR] Unsupported by FBO implementation.\n");
        return false;

    default:
        DBG_PRINT("[ERROR] Unknown error. %d\n",status);
        return false;
    }
}



GLRenderer::GLRenderer()
{
	if (initializeSharedMemory()==0) {
		DBG_PRINT("All ok!\n");
	} else {
		DBG_PRINT("Error initializing Shared memory\n");
	}

	initOpenGLES2Parser();

	viewportWidth = 0;
	viewportHeight = 0;

}

int GLRenderer::initializeSharedMemory() {
	int fileSize;

	thePosixSharedMemory = new PosixSharedMemory("qemu_gles2emulator_inputBuffer");
	while (thePosixSharedMemory->fileDescriptor < 0) {
		DBG_PRINT("waiting for shared memory\n");
		delete thePosixSharedMemory;
		pthread_yield();
		thePosixSharedMemory = new PosixSharedMemory("qemu_gles2emulator_inputBuffer");
	}

	thePosixSharedMemory->mapMemory(totalBufferSize);
	if (thePosixSharedMemory->mmappedAddress < 0)
	{
		DBG_PRINT("Could not mmap file!");
		return -1;
	}


	theAndroidSharedMemory = new PosixSharedMemory("qemu_device_ram1");
	if (theAndroidSharedMemory->fileDescriptor < 0)
	{
		DBG_PRINT("Could not find Android shared memory file!\n");
		return theAndroidSharedMemory->fileDescriptor;
	}
	fileSize = theAndroidSharedMemory->GetSharedMemoryFileSize(theAndroidSharedMemory->fileDescriptor);
	DBG_PRINT("Mapping Android shared memory of size: %d", fileSize);
	theAndroidSharedMemory->mapMemory(fileSize);
	if (theAndroidSharedMemory->mmappedAddress < 0)
	{
		DBG_PRINT("Could not mmap Android shared memory file!\n");
		return -1;
	}

	thePosixSemaphore = new PosixSemaphore("qemu_virtualdevice1_semaphore");
	theResetSemaphore = new PosixSemaphore("qemu_virtualdevice1_systemReset_semaphore");

	currentBufferPointer = 0;
	theCopyBuffer = 0;

	do {} while (thePosixSemaphore->tryWait() == 0);
	do {} while (theResetSemaphore->tryWait() == 0);

	return 0;
} 

GLRenderer::~GLRenderer()
{
	// TODO Auto-generated destructor stub
    glDeleteLists(pbufferList, 1);
}


void GLRenderer::initializeGL()
{


    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable (GL_BLEND);
    glShadeModel (GL_FLAT);
    glEnable (GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	/* Reserve 1 meg for our screen flip buffer */
	theCopyBuffer = (char *)malloc(1024*1024);
	if (theCopyBuffer == 0) DBG_PRINT("Yikes!  Could not allocate memory for image flip buffer!\n");

    glClearColor(1.0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();

}


int GLRenderer::testReset()
{
	if (theResetSemaphore->tryWait() == 0 )
	{
		DBG_PRINT("System reset requested!  Resetting buffers!\n");
		theSurfaces[0].surfacePhysicalAddress = 0;
		theSurfaces[1].surfacePhysicalAddress = 0;
		currentBufferPointer = 0;
		do {} while (thePosixSemaphore->tryWait() == 0);
		return 1;
	} else {
		return 0;	
	}
}

void GLRenderer::tranferGLImageBufferFlipped_Y(char *sourceBuffer, char *addressBase, theSurfaceStruct *thisSurface)
{
unsigned int size, stride, bytesPerLine, bytesPerPixel;
char *theCopyAddress;
int i;


	bytesPerPixel = 2;				// These will be read from the surface structure when available.
	stride = 0;
	bytesPerLine = 0;

	size = thisSurface->width * thisSurface->height * bytesPerPixel;
	if (sourceBuffer)
	{
		bytesPerLine = bytesPerPixel * thisSurface->width;
		stride = bytesPerLine;		/* At the moment not reading current stride. */
		glReadPixels(0, 0, thisSurface->width, thisSurface->height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (GLvoid *) sourceBuffer);
		theCopyAddress = addressBase + thisSurface->surfacePhysicalAddress + (size - stride);

		for (i = 0; i < thisSurface->height; i++)
		{
			memcpy((void *)theCopyAddress, (void *)sourceBuffer, bytesPerLine);
			sourceBuffer += stride;
			theCopyAddress -= stride;
		}
	}
}

void GLRenderer::GLEventLoop()
{
	int payloadSize;
	char tempDataArray[256];
	void *theBlottoPointer;
	int loop =0;

	while (true)
	{


		testReset();

		thePosixSemaphore->wait();

		{
			int i;
			int cmd, virtualDeviceMagicNumber;
			int subCommand, magicData, surfaceEnumerator;

			theBufferPointer = (int *)thePosixSharedMemory->mmappedAddress;
			
			cmd = 0;
			subCommand = 0;

			for (i = 0; ((i < 512) && (cmd != 3 && subCommand != 3)); i++)
			{
				/* In case we get a signal mid-parse. */
				if (testReset()) break;

				fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
				virtualDeviceMagicNumber = *(int *)tempDataArray;
				int retries = 0;
				while (virtualDeviceMagicNumber != 0x4703F322 && retries < 60){
					currentBufferPointer -= 3; 
					fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
					virtualDeviceMagicNumber = *(int *)tempDataArray;
					retries++;
				}
				if (retries>0) {
					printf("resync shifted: %d\n", retries);
				} else {
					DBG_PRINT("ok\n");
				}
				fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
				payloadSize = *(int *)tempDataArray;
				fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
				cmd = *(int *)tempDataArray;
				fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
				int seq = *(int *)tempDataArray;
				DBG_PRINT("sequence# %d\n",seq);
				
				if (virtualDeviceMagicNumber == GLES2_DEVICE_HEADER)
				{
					//DBG_PRINT("Current buffer pointer: %d\n", currentBufferPointer);
					switch (cmd)
					{
						case 1:
						{
							DBG_PRINT("Received GLES1 command.  Payload size: %d.  Parsing!\n", payloadSize);
							parseGLES1_1Command(theBufferPointer, &currentBufferPointer, totalBufferSize);
							break;
						}
						case 2:
						{
							DBG_PRINT("Received GLES2 command, payload size: %d\n", payloadSize);
							parseGLES2Command(theBufferPointer, &currentBufferPointer, totalBufferSize, payloadSize);
							break;
						}
						case 3:
						{
							DBG_PRINT("Received EGL1.4 command, payload size: %d\n", payloadSize);
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							subCommand = *(int *)tempDataArray;
							//DBG_PRINT("        Subcommand: %d\n", subCommand);

						switch (subCommand)
						{
							case 1:
							{
								int whichSurface;

								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								whichSurface = *(int *)tempDataArray;
								if (whichSurface > 1 )
								{
									DBG_PRINT("Surface out of range!\n");
									break;
								}
								theSurfaces[whichSurface].surfaceEnumerator = whichSurface;
								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								theSurfaces[whichSurface].pid = *(int *)tempDataArray;
								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								theSurfaces[whichSurface].surfacePhysicalAddress = *(unsigned int *)tempDataArray;
								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								theSurfaces[whichSurface].surfaceVirtualAddress = *(unsigned int *)tempDataArray;
								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								theSurfaces[whichSurface].width = *(unsigned int *)tempDataArray;
								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								theSurfaces[whichSurface].height = *(unsigned int *)tempDataArray;
								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								theSurfaces[whichSurface].pixelFormat = *(int *)tempDataArray;
								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								theSurfaces[whichSurface].pixelType = *(int *)tempDataArray;
								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								theSurfaces[whichSurface].stride = *(int *)tempDataArray;
								DBG_PRINT("Surface identifier: %d ,phys addr: 0x%x, width: %d, height: %d\n, stride: %d", theSurfaces[whichSurface].surfaceEnumerator, theSurfaces[whichSurface].surfacePhysicalAddress, theSurfaces[whichSurface].width, theSurfaces[whichSurface].height, theSurfaces[whichSurface].stride);
								if (viewportWidth != theSurfaces[whichSurface].width || viewportHeight != theSurfaces[whichSurface].height) {
									viewportWidth =  theSurfaces[whichSurface].width;
									viewportHeight =  theSurfaces[whichSurface].height;
									glViewport(0,0,viewportWidth, viewportHeight);  
								}
								break;
							}

							case 3:
							{
								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								magicData = *(int *)tempDataArray;
								fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
								surfaceEnumerator = *(int *)tempDataArray;
								DBG_PRINT("        SYNC command, magic: 0x%x, surface number: 0x%x\n", magicData, surfaceEnumerator);
								if (surfaceEnumerator > 1 )
								{
									DBG_PRINT("Surface out of range!\n");
									break;
								}

								if (theSurfaces[surfaceEnumerator].surfacePhysicalAddress != 0)
								{

									tranferGLImageBufferFlipped_Y((char *)theCopyBuffer, (char *)theAndroidSharedMemory->mmappedAddress, &theSurfaces[surfaceEnumerator]);

									glutSwapBuffers();
									i = 99999;		/* Cause an exit of the loop, we're done with a sync. */
								}
							
								break;
							}
							
							break;
						}


							break;
						}

						default:
						{
							DBG_PRINT("command is not GLES11 and GLES20\n");
							break;
						}
					}

				}
				else
				{
					DBG_PRINT("Command does not start with Magic Number!!! Ignoring the command\n");
				}
			}

		}
	}


}

