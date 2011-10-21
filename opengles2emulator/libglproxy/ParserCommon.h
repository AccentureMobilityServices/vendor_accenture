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
#ifndef PARSERCOMMON_H_
#define PARSERCOMMON_H_

#include <stdio.h>
#include "glheaders.h" 

typedef GLclampf GLclampfValue;
typedef GLclampf GLclampxValue;


int parse_glClear(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glClearColorf(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glClearColorx(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);

int parse_nullFunction(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);


void fetchBufferWrappedBytes(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize, void *targetBuffer, int numberOfBytes);
GLbitfield fetch_GLbitfield_32(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
GLclampfValue fetch_GLclampfValue_32(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
GLclampxValue fetch_GLclampxValue_32(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
GLenum fetch_GLenum(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
GLuint fetch_GLuint(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
GLint fetch_GLint(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
GLsizei fetch_GLsizei(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
GLchar* fetch_GLstring(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize, int len);



#endif /* PARSERCOMMON_H_ */
