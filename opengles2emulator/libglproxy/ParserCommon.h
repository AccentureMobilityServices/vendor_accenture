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

class ParserBuffer {
public:
	ParserBuffer(int bufferSize);

	void setBufferSize(int bufferSize);
	void resetBuffer();
	void markPos();
	void returnToMark();
	void advance(int size);
	void backward(int size);

	void fetchBufferWrappedBytes(void *targetBuffer, int numberOfBytes);
	GLbitfield fetch_GLbitfield_32();
	GLclampfValue fetch_GLclampfValue_32();
	GLclampxValue fetch_GLclampxValue_32();
	GLenum fetch_GLenum();
	GLuint fetch_GLuint();
	GLint fetch_GLint();
	GLsizei fetch_GLsizei();
	void fetch_GLstring(int len, GLchar* buffer);
	GLsizeiptr fetch_GLsizeiptr();

	char* getBufferPointer() {return theBufferAddress + locationInBuffer;}

private:
	char* theBufferAddress;
	unsigned int locationInBuffer;
	int bufferSize;
	int mark;
};

#endif /* PARSERCOMMON_H_ */
