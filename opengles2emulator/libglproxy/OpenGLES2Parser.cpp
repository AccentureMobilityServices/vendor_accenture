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

#include "OpenGLES2Parser.h"
#include "ParserCommon.h"
#include "GLES2.h"
#include "gles2_emulator_constants.h"
#include "AttribPointer.h"
#include "debug.h"
#include <string.h>
/*
 * These should be global.
 */


int (*parserGLES2FunctionPointers[NUMBER_OF_GLES2_POINTERS])(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);

GLuint* tokens;
void* indicesPtr;
AttribPointer** attribs;

int initOpenGLES2Parser(void)
{
	int i;

	tokens = new GLuint[256];	
	for (i=0;i<256;i++) {
		tokens[i] = -1;

	}
	indicesPtr = NULL;

	for (i = 0; i < NUMBER_OF_GLES2_POINTERS; i++)
	{
		parserGLES2FunctionPointers[i] = parse_nullFunction;
	}

	attribs = new AttribPointer*[MAX_ATTRIBS];
	//set all the pointers to NULL
	for (int i=0; i<MAX_ATTRIBS;i++) {
		attribs[i] = NULL;
	}

	parserGLES2FunctionPointers[GLCLEAR] = parse_glClear;
	parserGLES2FunctionPointers[GLCLEARCOLORX] = parse_glClearColorx;
	parserGLES2FunctionPointers[GLCLEARCOLORF] = parse_glClearColorf;
	parserGLES2FunctionPointers[GLCREATEPROGRAM] = parse_glCreateProgram;
	parserGLES2FunctionPointers[GLCREATESHADER] = parse_glCreateShader;
	parserGLES2FunctionPointers[GLSHADERSOURCE] = parse_glShaderSource;
	parserGLES2FunctionPointers[GLUSEPROGRAM] = parse_glUseProgram;
	parserGLES2FunctionPointers[GLVERTEXATTRIBPOINTER] = parse_glVertexAttribPointer;
	parserGLES2FunctionPointers[GLENABLEVERTEXATTRIBARRAY] = parse_glEnableVertexAttribArray;
	parserGLES2FunctionPointers[GLATTACHSHADER] = parse_glAttachShader;
	parserGLES2FunctionPointers[GLLINKPROGRAM] = parse_glLinkProgram;
	parserGLES2FunctionPointers[GLGETPROGRAMINFOLOG] = parse_glGetProgramInfoLog;
	parserGLES2FunctionPointers[GLDELETEPROGRAM] = parse_glDeleteProgram;
	parserGLES2FunctionPointers[GLGETATTRIBLOCATION] = parse_glGetAttribLocation;
	parserGLES2FunctionPointers[GLCOMPILESHADER] = parse_glCompileShader;
	parserGLES2FunctionPointers[GLDRAWARRAYS] = parse_glDrawArrays;
	parserGLES2FunctionPointers[GLDRAWELEMENTS] = parse_glDrawElements;
	parserGLES2FunctionPointers[GLGETUNIFORMLOCATION] = parse_glGetUniformLocation;
	parserGLES2FunctionPointers[GLUNIFORM3FV] = parse_glUniform3fv;
	parserGLES2FunctionPointers[GLUNIFORMMATRIX4FV] = parse_glUniformMatrix4fv;
	parserGLES2FunctionPointers[GLDISABLE] = parse_glDisable;
	parserGLES2FunctionPointers[GLENABLE] = parse_glEnable;
	parserGLES2FunctionPointers[GLVIEWPORT] = parse_glViewport;
	return 0;

}

int parseGLES2Command(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize, int payloadSize)
{
int theGLCommandToProcess;

	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, &theGLCommandToProcess, sizeof(int));

	unsigned int bufferPos = *locationInBuffer;
	(*locationInBuffer)+= payloadSize;
	(*locationInBuffer)%= bufferSize;

	DBG_PRINT("(%s)  Command: %d]\n", __FUNCTION__, theGLCommandToProcess);
	if (theGLCommandToProcess >= NUMBER_OF_GLES2_POINTERS)
	{
		return -1;
	}


	return parserGLES2FunctionPointers[theGLCommandToProcess](theBufferAddress, &bufferPos, bufferSize);

}



