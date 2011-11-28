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
#elif defined WIN32
#else
#include <pthread.h>
#endif
#include <unistd.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#include "glheaders.h"
#include "GLRenderer.h"


#include "OpenGLES2Parser.h"

#include "gles2_emulator_constants.h"
#include "debug.h"

#define ADDRESS     "/tmp/glproxy-socket"  /* addr to connect */

#ifdef WIN32
bool isErrorRecoverable() {
	int error = WSAGetLastError();
	if (error == WSAEINTR) {
		return true;
	}
	return false;
}
#else

bool isErrorRecoverable() {
	perror("recoverable?");
	if (errno == EAGAIN || errno == EINTR) {
		return true;
	}
	return false;
}

#endif

int socketRead(int fd, char* ptr, int bytesToRead) {
	int bytesRead = 0;
	while (bytesToRead > 0) {
		int bytes = recv(fd, ptr, bytesToRead, 0);
		if (bytes==-1) {
			if (isErrorRecoverable())
				continue;
			else {
				bytesRead = -1;
				break;
			}
		}
		if (bytes==0) {
			bytesRead = 0;
			break;
		}

		bytesToRead -= bytes;
		bytesRead += bytes;
		ptr+= bytes;
	}
	return bytesRead;
} 


int socketWrite(int fd, char* ptr, int bytesToWrite) {
	int bytesWritten = 0;
	while (bytesToWrite > 0) {
		int bytes = send(fd, ptr, bytesToWrite, 0);
		if (bytes==-1) {
			if (isErrorRecoverable())
				continue;
			else {
				bytesWritten = -1;
				break;
			}
		}

		bytesToWrite -= bytes;
		bytesWritten += bytes;
		ptr+= bytes;
	}
	DBGPRINT(">>> socket write %d\n", bytesWritten);
	return bytesWritten;
} 

GLRenderer::GLRenderer()
{
	if (initializeSharedMemory()==0) {
		DBGPRINT("All ok!\n");
	} else {
		DBGPRINT("Error initializing Shared memory\n");
	}

	buffer = new ParserBuffer(100*1024);
	gl2parser = new GLES2Parser(*buffer);
	gl2parser->setSharedMemory(theAndroidSharedMemory);

	socketfd = -1;
	acceptfd = -1;

#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2,2);
	int error = WSAStartup(wVersionRequested, &wsaData);
	if (error != 0 ) {
		printf("Could not start windows sockets\n");
		exit(1);
	}
#endif

}

int GLRenderer::initializeSharedMemory() {
	int fileSize;

	theAndroidSharedMemory = new SharedMemory("qemu_device_ram1");
	while (!theAndroidSharedMemory->initialised())
	{
#ifdef WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
		delete theAndroidSharedMemory;
		theAndroidSharedMemory = new SharedMemory("qemu_device_ram1");
		DBGPRINT("waiting for android shared memory!\n");
	}
#ifdef __APPLE__
	// temporary workaround - it isn't possible to open the shared memory as a file on macosx
	fileSize = 100663296;
#elif defined WIN32
	fileSize = 100663296;
#else
	fileSize = theAndroidSharedMemory->GetSharedMemoryFileSize();
#endif

	DBGPRINT("Mapping Android shared memory of size: %d\n", fileSize);
	theAndroidSharedMemory->mapMemory(fileSize);
	if (theAndroidSharedMemory->getMappedAddress() < 0)
	{
		DBGPRINT("Could not mmap Android shared memory file!\n");
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
	return socketWrite(acceptfd,(char*) &returnAddress, sizeof(void*)); 
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
		DBGPRINT("Could not allocate memory for image flip buffer!\n");
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
	if (socketfd == -1 ) 
	{
		if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("client: socket");
			exit(1);
		}

		unsigned int sendSize = SOCKET_SNDBUF_SIZE;
		unsigned int recvSize = SOCKET_RCVBUF_SIZE;
		
		setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, (char *) &sendSize, sizeof(sendSize));
		setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char *) &recvSize, sizeof(recvSize));

		/*
		 * Create the address we will be connecting to.
		 */
		saun = (sockaddr_in*)calloc(1, sizeof(sockaddr_in));
		
 		saun->sin_family = AF_INET;
		saun->sin_addr.s_addr = inet_addr("127.0.0.1");
		saun->sin_port = 2222;

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
		len = sizeof(sockaddr_in);

		int err;
		err = bind(socketfd, (sockaddr*) saun, len);
		if (err < 0) {
			perror("bind problem");
			return;
		}

		/*
		 * Listen on the socket.
		 */
		err = listen(socketfd, 5);
		printf("listen on socket\n");

	}
	while (acceptfd == -1) {
		struct sockaddr_in peer_addr;
		socklen_t fromlen = sizeof(peer_addr);
		acceptfd = accept(socketfd, (sockaddr*)saun, &fromlen);
		if (acceptfd != -1) {
			printf("accept connection\n");
		}
	}
	free(saun);
	int bytesRead=0;
	do {
#ifdef __APPLE__
		glutCheckLoop();
#else
		glutMainLoopEvent();
#endif
		buffer->resetBuffer();
		int bytesToRead = sizeof(command_control);
		bytesRead = socketRead(acceptfd, buffer->getBufferPointer(), bytesToRead);
		buffer->resetBuffer();
		DBGPRINT("bytes read %d\n", bytesRead);
		if (bytesRead > 0) {
			int magic = buffer->fetch_GLint();
			int payloadSize = buffer->fetch_GLint();
			int seqNumber = buffer->fetch_GLint();
			DBGPRINT("magic: %08x\n", magic);
			if (magic != 0x4703f322) {
				DBGPRINT ("error: header wrong: 0x%08x\n", magic);
				exit(0);
			}
			DBGPRINT("payload size: %0d\n", payloadSize);
			DBGPRINT("seq #: %0d\n", seqNumber);
			buffer->setBufferSize(payloadSize+sizeof(command_control));
			buffer->resetBuffer();
			buffer->advance(sizeof(command_control));
			bytesToRead = payloadSize;
			if (bytesToRead > 0) {
				bytesRead = socketRead(acceptfd, buffer->getBufferPointer(), bytesToRead);
				DBGPRINT("bytes read %d, payloadSize %d\n", bytesRead, payloadSize);
			}
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
					DBGPRINT("resync shifted: %d\n", retries);
				} else {
					DBGPRINT("ok\n");
				}
				payloadSize = buffer->fetch_GLint();
				int seqNumber = buffer->fetch_GLint();
				cmd = buffer->fetch_GLint();
				int contextID = buffer->fetch_GLint();
				int retVal = buffer->fetch_GLint();
				GLproxyContext* context = findContext(contextID);
				context->switchToContext();

				void* returnAddress = NULL;
				void *physAddress = NULL;


				if (retVal != 0)
				{
					DBGPRINT("return value requested: 0x%x\n", retVal);
					physAddress = (void *)retVal;
					returnAddress = ((char*)(theAndroidSharedMemory->getMappedAddress()))+retVal;
				}

				if (virtualDeviceMagicNumber == GLES2_DEVICE_HEADER)
				{
					//DBGPRINT("Current buffer pointer: %d\n", currentBufferPointer);
					switch (cmd)
					{
						case 1:
							{
								DBGPRINT("Received GLES1 command.  Payload size: %d.  Parsing!\n", payloadSize);
								break;
							}
						case 2:
							{
								DBGPRINT("Received GLES2 command, payload size: %d\n", payloadSize);
								gl2parser->setContext(context);
								gl2parser->parseGLES2Command(payloadSize, (void*)returnAddress);
								if (returnAddress != NULL) {
									DBGPRINT("Sending return value: 0x%x\n", (unsigned int)physAddress);
									sendReturnReady(physAddress);
								}
								break;
							}
						case 3:
							{
								DBGPRINT("Received EGL1.4 command, payload size: %d\n", payloadSize);
								subCommand = buffer->fetch_GLint();
								DBGPRINT("        Subcommand: %d\n", subCommand);

								switch (subCommand)
								{
									case EGL_SURFACE:
										{
											int whichSurface;

											whichSurface = buffer->fetch_GLint();
											theSurfaceStruct* surface = context->getSurface();

											//printf("context# %08x - ",contextID);
											surface->surfaceEnumerator = whichSurface;
											surface->pid = buffer->fetch_GLint();
											surface->surfacePhysicalAddress = buffer->fetch_GLint();
											surface->surfaceVirtualAddress = buffer->fetch_GLuint();
											surface->width = buffer->fetch_GLuint();
											surface->height = buffer->fetch_GLuint();
											surface->pixelFormat = buffer->fetch_GLint();
											surface->pixelType = buffer->fetch_GLint();
											surface->stride = buffer->fetch_GLint();
											break;
										}
									case EGL_DESTROYCONTEXT:
										{
											DBGPRINT("destroy context\n");
											deleteContext(contextID);
										}
										break;
									case EGL_SYNC:
										{
											magicData = buffer->fetch_GLint();
											surfaceEnumerator = buffer->fetch_GLint();

											theSurfaceStruct* surface = context->getSurface();
											DBGPRINT("        SYNC command, magic: 0x%x, surface number: 0x%x\n", magicData, surfaceEnumerator);

											if (surface->surfacePhysicalAddress != 0)
											{
												transferGLImageBufferFlipped_Y( (char *)theAndroidSharedMemory->getMappedAddress(), surface);
												glutSwapBuffers();
											}
											if (returnAddress != NULL) {
												*(int*)returnAddress = 12345;
												DBG_PRINT("Sending return value: 0x%x\n", (unsigned int)physAddress);
												sendReturnReady(physAddress);
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
					DBGPRINT("Command does not start with Magic Number!!! Ignoring the command\n");
				}
			}

		}

	} while (bytesRead>0);
	close(acceptfd);
	DBGPRINT("socket connection broken\n");
	acceptfd = -1;

}

