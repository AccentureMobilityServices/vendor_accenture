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

#ifndef GLRENDERER_H_
#define GLRENDERER_H_
#include <GL/gl.h>
#include "PosixSemaphore.h"
#include "PosixSharedMemory.h"
#include <string.h>

class GLRenderer
{

	typedef struct
	{
		int	surfaceEnumerator;
		int pid;
		unsigned int surfacePhysicalAddress;
		unsigned int surfaceVirtualAddress;
		unsigned int width;
		unsigned int height;
		int pixelFormat;
		int pixelType;
		unsigned int stride;
	} theSurfaceStruct;

	  struct
	  {
		  GLenum target;
		  GLint level;
		  GLint internalformat;
		  GLsizei width;
		  GLsizei height;
		  GLint border;
		  GLenum format;
		  GLenum type;
		  const GLvoid *pixels;
	  } theglTexImage2Dstruct;

public:
	GLRenderer();
	virtual ~GLRenderer();

    void initializeGL();
    void resizeGL(int width, int height);
    int testReset();
	void GLEventLoop();
private:

	int initializeSharedMemory();
	void tranferGLImageBufferFlipped_Y(char *sourceBuffer, char *addressBase, theSurfaceStruct *thisSurface);
	  GLfloat oneColour, xRotation, yRotation, zRotation;
	  PosixSemaphore *thePosixSemaphore;
	  PosixSemaphore *theResetSemaphore;
	  PosixSharedMemory *thePosixSharedMemory;
	  PosixSharedMemory *theAndroidSharedMemory;

	  unsigned int currentBufferPointer;
	  int theMagicNumber, theGLcommand;

	  int * theBufferPointer;
	  GLclampf *theGLclampfValue;
	  GLclampf *theGLclampxValue;
	  GLbitfield *theGLbitfieldValue;

	GLuint fb;
	GLuint color_rb;
      GLuint pbufferList;
      GLuint cubeTexture;

	void *theCopyBuffer;
	theSurfaceStruct theSurfaces[2];

	int viewportWidth, viewportHeight;
};

#endif /* GLRENDERER_H_ */


