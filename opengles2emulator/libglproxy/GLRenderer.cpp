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
#ifdef __APPLE__
#include <sched.h>
#else
#include <pthread.h>
#endif
#include <unistd.h>

#include "glheaders.h"
#include "GLRenderer.h"


#include "OpenGLES1_1Parser.h"
#include "OpenGLES2Parser.h"

#include "gles2_emulator_constants.h"
#include "debug.h"

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

	thePosixSharedMemory = new PosixSharedMemory("qemu_vd1_inputBuffer");
	while (thePosixSharedMemory->fileDescriptor < 0) {
		DBG_PRINT("waiting for shared memory\n");
		delete thePosixSharedMemory;
#ifdef __APPLE__
		sched_yield();
#else		
		pthread_yield();
#endif
		thePosixSharedMemory = new PosixSharedMemory("qemu_vd1_inputBuffer");
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
#ifdef __APPLE__
	// temporary workaround - it isn't possible to open the shared memory as a file on macosx
	fileSize = 100663296;
#else
	fileSize = theAndroidSharedMemory->GetSharedMemoryFileSize(theAndroidSharedMemory->fileDescriptor);
#endif

	DBG_PRINT("Mapping Android shared memory of size: %d\n", fileSize);
	theAndroidSharedMemory->mapMemory(fileSize);
	if (theAndroidSharedMemory->mmappedAddress < 0)
	{
		DBG_PRINT("Could not mmap Android shared memory file!\n");
		return -1;
	}

	thePosixSemaphore = new PosixSemaphore("qemu_vd1_semaphore");
	theResetSemaphore = new PosixSemaphore("qemu_vd1_systemReset_sem");

	currentBufferPointer = 0;
	theCopyBuffer = 0;

	theOutputIPCMessageQueue = new IPC_Messaging;
	theOutputIPCMessageQueue->open("/gles2emulator_msgQInput", O_CREAT | O_RDWR) ;
	sendReset();
	do {} while (thePosixSemaphore->tryWait() == 0);
	do {} while (theResetSemaphore->tryWait() == 0);

	return 0;
} 

GLRenderer::~GLRenderer()
{
	// TODO Auto-generated destructor stub
	glDeleteLists(pbufferList, 1);
}

GLproxyContext* GLRenderer::findContext(int contextID) 
{
	//first try to find existing context
	vector<GLproxyContext*>::iterator iter;
	GLproxyContext* context = NULL;
	for (iter = glcontexts.begin(); iter != glcontexts.end(); iter++) {
		GLproxyContext* cntx = *iter;
		if (cntx->isContext(contextID)) {
			context = cntx;
			break;
		}
	}
	if (context == NULL) {
		context = new GLproxyContext(contextID);
		context->createContext();
		glcontexts.push_back(context);
	}
	return context;
}

void GLRenderer::deleteContext(int contextID) 
{
	vector<GLproxyContext*>::iterator iter;
	GLproxyContext* context = NULL;
	for (iter = glcontexts.begin(); iter != glcontexts.end(); iter++) {
		GLproxyContext* context = *iter;
		if (context->isContext(contextID)) {
			context->destroyContext();
			delete context;
			glcontexts.erase(iter);
			break;
		}
	}
}


void GLRenderer::initializeGL()
{
	glEnable (GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0, 0, 0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();

}


int GLRenderer::testReset()
{
	if (theResetSemaphore->tryWait() == 0)
	{
			printf("System reset!\n");
			currentBufferPointer = 0;
			theSurfaces[0].surfacePhysicalAddress = 0;
			theSurfaces[1].surfacePhysicalAddress = 0;
			do {} while (theResetSemaphore->tryWait() == 0);
			return 1;
	} else {
		return 0;
	}
}


int GLRenderer::sendReset()
{
	static char theMessage[] = {0x22, 0xF3, 0x03, 0x47,		// Magic number.
									4, 0x00, 0x00, 0x00,	// Command data length.
									0x03, 0x00, 0x00, 0x00,	// Command (3 = EGL 1.4).
									0xff, 0xff, 0xff, 0xff,	// Sequence Number
									0x08, 0x00, 0x00, 0x00};// Subcommand (8 = RESET_HOST_BUFFER_POINTER).

	theOutputIPCMessageQueue->send_message((char*)&theMessage, sizeof(theMessage), 0);
	do {} while (thePosixSemaphore->tryWait() == 0);
	currentBufferPointer = 0;
	return 0;
}


void GLRenderer::transferGLImageBufferFlipped_Y(char *addressBase, theSurfaceStruct *thisSurface)
{
	unsigned int size, stride, bytesPerLine, bytesPerPixel;
	char *theCopyAddress;
	int i;

	bytesPerPixel = 2;				// These will be read from the surface structure when available.
	stride = 0;
	bytesPerLine = 0;

	size = thisSurface->width * thisSurface->height * bytesPerPixel;
	bytesPerLine = bytesPerPixel * thisSurface->width;
	stride = bytesPerLine;		/* At the moment not reading current stride. */

	theCopyBuffer = realloc(theCopyBuffer, thisSurface->height*stride);
	if (theCopyBuffer == NULL) { 
		DBG_PRINT("Could not allocate memory for image flip buffer!\n");
		return;
	}

	glReadPixels(0, 0, thisSurface->width, thisSurface->height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (GLvoid *) theCopyBuffer);
	theCopyAddress = addressBase + thisSurface->surfacePhysicalAddress + (size - stride);
	char* sourceBuffer = (char*)theCopyBuffer;
	for (i = 0; i < thisSurface->height; i++)
	{
		memcpy(theCopyAddress, sourceBuffer, bytesPerLine);
		sourceBuffer += stride;
		theCopyAddress -= stride;
	}

}

void GLRenderer::GLEventLoop()
{
	int payloadSize;
	char tempDataArray[256];
	void *theBlottoPointer;
	int loop =0;

	{


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
				fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
				virtualDeviceMagicNumber = *(int *)tempDataArray;
				int retries = 0;
				if (virtualDeviceMagicNumber != 0x4703F322){
					printf("Sync header not found!  Sending host buffer reset!\n");
					sendReset();
					break;
				}
				fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
				payloadSize = *(int *)tempDataArray;
				fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
				cmd = *(int *)tempDataArray;
				fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
				int contextID = *(int *)tempDataArray;
				DBG_PRINT("context# %08x\n",contextID);
				GLproxyContext* context = findContext(contextID);
				context->switchToContext();
				
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
						case EGL_SURFACE:
						{
							int whichSurface;

							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							whichSurface = *(int *)tempDataArray;
							theSurfaceStruct* surface = context->getSurface(whichSurface);
							if (!surface )
							{
								DBG_PRINT("Surface out of range!\n");
								break;
							}

								
							surface->surfaceEnumerator = whichSurface;
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							surface->pid = *(int *)tempDataArray;
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							surface->surfacePhysicalAddress = *(unsigned int *)tempDataArray;
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							surface->surfaceVirtualAddress = *(unsigned int *)tempDataArray;
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							surface->width = *(unsigned int *)tempDataArray;
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							surface->height = *(unsigned int *)tempDataArray;
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							surface->pixelFormat = *(int *)tempDataArray;
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							surface->pixelType = *(int *)tempDataArray;
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							surface->stride = *(int *)tempDataArray;
							DBG_PRINT("Surface identifier: %d ,phys addr: 0x%x, width: %d, height: %d\n, stride: %d", surface->surfaceEnumerator, surface->surfacePhysicalAddress, surface->width, surface->height, surface->stride);
							break;
						}
						case EGL_DESTROYCONTEXT:
						{
							deleteContext(contextID);
						}
						break;
						case EGL_SYNC:
						{
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							magicData = *(int *)tempDataArray;
							fetchBufferWrappedBytes(theBufferPointer, &currentBufferPointer, totalBufferSize, tempDataArray, sizeof(int));
							surfaceEnumerator = *(int *)tempDataArray;

							theSurfaceStruct* surface = context->getSurface(surfaceEnumerator);
							DBG_PRINT("        SYNC command, magic: 0x%x, surface number: 0x%x\n", magicData, surfaceEnumerator);
							
							if (!surface )
							{
								DBG_PRINT("Surface out of range!\n");
								break;
							}
								
							if (surface->surfacePhysicalAddress != 0)
							{

								transferGLImageBufferFlipped_Y( (char *)theAndroidSharedMemory->mmappedAddress, surface);

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

