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
#include "glheaders.h"
#include "glproxy_context.h"
#include "SharedMemory.h"
#include <string>
#include <vector>

struct socketaddr_in;

using namespace std;

class GLES2Parser;
class ParserBuffer;

class GLRenderer
{

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

	void resizeGL(int width, int height);
	int sendReturnReady(void* returnAddress);
	int sendHostSync(int theValue);
	void GLEventLoop();
	void ParseData();

	GLproxyContext* findContext(int contextID);
	void deleteContext(int contextID);

 private:

	int initializeSharedMemory();
	void transferGLImageBufferFlipped_Y(char *addressBase, theSurfaceStruct *thisSurface);
	GLfloat oneColour, xRotation, yRotation, zRotation;
	SharedMemory *theAndroidSharedMemory;
	GLES2Parser* gl2parser;
	ParserBuffer* buffer;
	vector<GLproxyContext*> glcontexts;

	int theMagicNumber, theGLcommand;


	void *theCopyBuffer;

	int i, len, socketfd, acceptfd;
	struct sockaddr_in* saun;
};

#endif /* GLRENDERER_H_ */


