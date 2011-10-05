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

#include "ParserCommon.h"
#include "debug.h"

/*
 * The GL rendering functions.
 */

char tempDataStorage[256];

int
parse_glClear(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
GLbitfield testField;

		DBG_PRINT("(%s)\n", __FUNCTION__);
		glClear(fetch_GLbitfield_32(theBufferAddress, locationInBuffer, bufferSize));

		return 0;
}

int
parse_glClearColorf(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
		DBG_PRINT("(%s)\n", __FUNCTION__);

		
		GLfloat r = fetch_GLclampfValue_32(theBufferAddress, locationInBuffer, bufferSize);
		GLfloat g = fetch_GLclampfValue_32(theBufferAddress, locationInBuffer, bufferSize);
		GLfloat b = fetch_GLclampfValue_32(theBufferAddress, locationInBuffer, bufferSize);
		GLfloat a = fetch_GLclampfValue_32(theBufferAddress, locationInBuffer, bufferSize);

		DBG_PRINT("color %f %f %f %f\n",r,g,b,a);	 
		glClearColor(r,g,b,a);
		return 0;
}

int
parse_glClearColorx(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
		DBG_PRINT("(%s)\n", __FUNCTION__);
		glClearColor(fetch_GLclampxValue_32(theBufferAddress, locationInBuffer, bufferSize),
		fetch_GLclampxValue_32(theBufferAddress, locationInBuffer, bufferSize),
		fetch_GLclampxValue_32(theBufferAddress, locationInBuffer, bufferSize),
		fetch_GLclampxValue_32(theBufferAddress, locationInBuffer, bufferSize));
		return 0;
}

int
parse_nullFunction(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
		DBG_PRINT("(%s)\n", __FUNCTION__);
		return -1;
}


/*
 * Data access support functions - should be as a separate file.
 */

void fetchBufferWrappedBytes(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize, void *targetBuffer, int numberOfBytes)
{
int i;
unsigned char *destAddress = (unsigned char *)targetBuffer, *sourceAddress = (unsigned char*)theBufferAddress;

	for (i = 0; i < numberOfBytes; i++)
	{
		destAddress[i] = sourceAddress[((*locationInBuffer)++)];
		(*locationInBuffer) %= bufferSize;
	}
}

GLbitfield
fetch_GLbitfield_32(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
GLbitfield theValue;

	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, tempDataStorage, sizeof(GLbitfield));
	theValue = *(GLbitfield *)tempDataStorage;
	return theValue;
}

GLclampfValue
fetch_GLclampfValue_32(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
GLclampfValue theValue;

	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, tempDataStorage, sizeof(GLclampfValue));
	theValue = *(GLclampfValue *)tempDataStorage;
	return theValue;
}

GLclampxValue
fetch_GLclampxValue_32(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
GLclampxValue theValue;

	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, tempDataStorage, sizeof(GLclampxValue));
	theValue = *(GLclampxValue *)tempDataStorage;
	return theValue;
}

GLenum fetch_GLenum(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	GLenum theValue;
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, tempDataStorage, sizeof(GLenum));
	theValue = *(GLenum *)tempDataStorage;
	return theValue;
}
GLuint fetch_GLuint(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	GLuint theValue;
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, tempDataStorage, sizeof(GLuint));
	theValue = *(GLuint *)tempDataStorage;
	return theValue;
}

GLint fetch_GLint(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	GLint theValue;
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, tempDataStorage, sizeof(GLuint));
	theValue = *(GLint *)tempDataStorage;
	return theValue;
}



GLsizei fetch_GLsizei(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	GLsizei theValue;
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, tempDataStorage, sizeof(GLsizei));
	theValue = *(GLsizei *)tempDataStorage;
	return theValue;
}

GLchar* fetch_GLstring(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize, int len)
{
	GLchar* theValue;
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, tempDataStorage, len);
	theValue = (GLchar *)tempDataStorage;
	return theValue;
}
