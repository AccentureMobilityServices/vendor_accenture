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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>

#include "glheaders.h"
#include "GLRenderer.h"


#include "OpenGLES2Parser.h"

#include "gles2_emulator_constants.h"
#include "debug.h"

#define ADDRESS     "/tmp/glproxy-socket"  /* addr to connect */


GLRenderer::GLRenderer()
{
	if (initializeSharedMemory()==0) {
		DBG_PRINT("All ok!\n");
	} else {
		DBG_PRINT("Error initializing Shared memory\n");
	}

	buffer = new ParserBuffer(100*1024);
	gl2parser = new GLES2Parser(*buffer);

	socketfd = -1;
	acceptfd = -1;

}

int GLRenderer::initializeSharedMemory() {
	int fileSize;

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


	theCopyBuffer = 0;

	return 0;
}

GLRenderer::~GLRenderer()
{
	delete gl2parser;
	delete buffer;
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


int GLRenderer::sendReturnReady(void* returnAddress)
{
	return write(acceptfd, &returnAddress, sizeof(void*)); 
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

ssize_t socket_get_available(int socket_fd)
{
        int available = -1;
        if (ioctl(socket_fd, FIONREAD, &available) < 0) {
                return -1;
        }

        return (ssize_t) available;
}

void GLRenderer::GLEventLoop()
{
	if (socketfd == -1 ) 
	{
		if ((socketfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			perror("client: socket");
			exit(1);
		}

		unsigned int sendSize = SOCKET_SNDBUF_SIZE;
		unsigned int recvSize = SOCKET_RCVBUF_SIZE;
		
		setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, (void *) &sendSize, sizeof(sendSize));
		setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (void *) &recvSize, sizeof(recvSize));

		/*
		 * Create the address we will be connecting to.
		 */
		saun.sun_family = AF_UNIX;
		strcpy(saun.sun_path, ADDRESS);

		/*
		 * Try to connect to the address.  For this to
		 * succeed, the server must already have bound
		 * this address, and must have issued a listen()
		 * request.
		 *
		 * The third argument indicates the "length" of
		 * the structure, not just the length of the
		 * socket name.
		 */
		len = sizeof(saun.sun_family) + strlen(saun.sun_path)+1;

		unlink(ADDRESS);

		int err;
		err = bind(socketfd, (sockaddr*) &saun, len);
		if (err < 0) {
			return;
		}

		/*
		 * Listen on the socket.
		 */
		err = listen(socketfd, 5);

		int flags = fcntl(socketfd, F_GETFL, 0);
		fcntl(socketfd, F_SETFL, flags);// | O_NONBLOCK);


	}
	if (acceptfd == -1) {
		struct sockaddr_un peer_addr;
		socklen_t fromlen = sizeof(peer_addr);
		acceptfd = accept(socketfd, (sockaddr*)&saun, &fromlen);
		DBG_PRINT("accept connection\n");
	}
	int bytesRead=0;
	while (bytesRead>=0) {
		while (socket_get_available(acceptfd)<=0) {
#ifdef __APPLE__
			glutCheckLoop();
#else
			glutMainLoopEvent();
#endif
		} 
		buffer->resetBuffer();
		int bytesToRead = sizeof(command_control);
		while (bytesToRead > 0) {
			//DBG_PRINT("bytes to read %d\n", bytesToRead);
			bytesRead = read(acceptfd, buffer->getBufferPointer(), bytesToRead);
			bytesToRead -= bytesRead;
			buffer->advance(bytesRead);
		}
		buffer->resetBuffer();
		DBG_PRINT("bytes read %d\n", bytesRead);
		if (bytesRead > 0) {
			int magic = buffer->fetch_GLint();
			int payloadSize = buffer->fetch_GLint();
			int seqNumber = buffer->fetch_GLint();
			DBG_PRINT("magic: %08x\n", magic);
			if (magic != 0x4703f322) {
				DBG_PRINT ("error: header wrong: 0x%08x\n", magic);
				exit(0);
			}
			DBG_PRINT("payload size: %0d\n", payloadSize);
			DBG_PRINT("seq #: %0d\n", seqNumber);
			buffer->setBufferSize(payloadSize+sizeof(command_control));
			buffer->resetBuffer();
			buffer->advance(sizeof(command_control));
			bytesToRead = payloadSize;
			while (bytesToRead > 0) {
				//DBG_PRINT("bytes to read %d\n", bytesToRead);
				bytesRead = read(acceptfd, buffer->getBufferPointer(), bytesToRead);
				bytesToRead -= bytesRead;
				buffer->advance(bytesRead);
			}
			DBG_PRINT("bytes read %d, payloadSize %d\n", bytesRead, payloadSize);
			buffer->resetBuffer();



			int cmd, virtualDeviceMagicNumber;
			int subCommand, magicData, surfaceEnumerator;

			cmd = 0;
			subCommand = 0;

			{
				virtualDeviceMagicNumber = buffer->fetch_GLint();
				int retries = 0;
				while (virtualDeviceMagicNumber != 0x4703F322 && retries < 60){
					virtualDeviceMagicNumber = buffer->fetch_GLint();
					retries++;
				}
				if (retries>0) {
					DBG_PRINT("resync shifted: %d\n", retries);
				} else {
					DBG_PRINT("ok\n");
				}
				payloadSize = buffer->fetch_GLint();
				int seqNumber = buffer->fetch_GLint();
				cmd = buffer->fetch_GLint();
				int contextID = buffer->fetch_GLint();
				int retVal = buffer->fetch_GLint();
				DBG_PRINT("context# %08x\n",contextID);
				GLproxyContext* context = findContext(contextID);
				context->switchToContext();

				void* returnAddress = NULL;
				void *physAddress = NULL;


				if (retVal != 0)
				{
					DBG_PRINT("return value requested: 0x%x\n", retVal);
					physAddress = (void *)retVal;
					returnAddress = ((char*)(theAndroidSharedMemory->mmappedAddress))+retVal;
				}

				if (virtualDeviceMagicNumber == GLES2_DEVICE_HEADER)
				{
					//DBG_PRINT("Current buffer pointer: %d\n", currentBufferPointer);
					switch (cmd)
					{
						case 1:
							{
								DBG_PRINT("Received GLES1 command.  Payload size: %d.  Parsing!\n", payloadSize);
								break;
							}
						case 2:
							{
								DBG_PRINT("Received GLES2 command, payload size: %d\n", payloadSize);
								gl2parser->setContext(context);
								gl2parser->parseGLES2Command(payloadSize, (void*)returnAddress);
								if (returnAddress != NULL) {
									DBG_PRINT("Sending return value: 0x%x\n", (unsigned int)physAddress);
									sendReturnReady(physAddress);
								}
								break;
							}
						case 3:
							{
								DBG_PRINT("Received EGL1.4 command, payload size: %d\n", payloadSize);
								subCommand = buffer->fetch_GLint();
								DBG_PRINT("        Subcommand: %d\n", subCommand);

								switch (subCommand)
								{
									case EGL_SURFACE:
										{
											int whichSurface;

											whichSurface = buffer->fetch_GLint();
											theSurfaceStruct* surface = context->getSurface(whichSurface);
											if (!surface )
											{
												DBG_PRINT("Surface out of range!  (%d)\n", whichSurface);
												break;
											}


											surface->surfaceEnumerator = whichSurface;
											surface->pid = buffer->fetch_GLint();
											surface->surfacePhysicalAddress = buffer->fetch_GLint();
											surface->surfaceVirtualAddress = buffer->fetch_GLuint();
											surface->width = buffer->fetch_GLuint();
											surface->height = buffer->fetch_GLuint();
											surface->pixelFormat = buffer->fetch_GLint();
											surface->pixelType = buffer->fetch_GLint();
											surface->stride = buffer->fetch_GLint();
											DBG_PRINT("Surface identifier: %d ,phys addr: 0x%x, width: %d, height: %d\n, stride: %d",
													surface->surfaceEnumerator,
													surface->surfacePhysicalAddress,
													surface->width,
													surface->height,
													surface->stride);
											break;
										}
									case EGL_DESTROYCONTEXT:
										{
											DBG_PRINT("destroy context\n");
											deleteContext(contextID);
										}
										break;
									case EGL_SYNC:
										{
											magicData = buffer->fetch_GLint();
											surfaceEnumerator = buffer->fetch_GLint();

											if (surfaceEnumerator > 1)
											{
												DBG_PRINT("Surface out of range!\n");
												break;
											}

											theSurfaceStruct* surface = context->getSurface(surfaceEnumerator);
											DBG_PRINT("        SYNC command, magic: 0x%x, surface number: 0x%x\n", magicData, surfaceEnumerator);

											if (surface->surfacePhysicalAddress != 0)
											{

												transferGLImageBufferFlipped_Y( (char *)theAndroidSharedMemory->mmappedAddress, surface);
												glutSwapBuffers();

												if (returnAddress != NULL) {
													*(int*)returnAddress = 12345;
													DBG_PRINT("Sending return value: 0x%x\n", (unsigned int)physAddress);
													sendReturnReady(physAddress);
												}
											}

											break;
										}


										break;
								}

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
	close(acceptfd);
	DBG_PRINT("socket connection broken\n");
	acceptfd = -1;

}

