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
#include <stdlib.h>
#include <string.h>
#include "ParserCommon.h"
#include "debug.h"

ParserBuffer::ParserBuffer(int bufferSize) {
	this->theBufferAddress = (char*)malloc(bufferSize);
	this->locationInBuffer = 0;
	this->bufferSize = bufferSize;
}

void ParserBuffer::resetBuffer() 
{
	locationInBuffer = 0;
}

void ParserBuffer::setBufferSize(int size) 
{
	if (size > bufferSize) {
		theBufferAddress = (char*)realloc(theBufferAddress, size);
		bufferSize = size;
	}
}

void ParserBuffer::backward(int size)
{
	locationInBuffer-=size;
	locationInBuffer%=bufferSize;
}

void ParserBuffer::markPos() 
{
	mark = locationInBuffer;
}

void ParserBuffer::returnToMark() 
{
	locationInBuffer = mark;
}

void ParserBuffer::advance(int size) 
{
	locationInBuffer += size;
	locationInBuffer %= bufferSize;
}

void ParserBuffer::fetchBufferWrappedBytes(void *targetBuffer, int numberOfBytes)
{
	int i;
	unsigned char *destAddress = (unsigned char *)targetBuffer, *sourceAddress = (unsigned char*)theBufferAddress;


	int charsLeft = bufferSize - locationInBuffer;
	int overflow = 0;
	if (numberOfBytes > charsLeft) 
	{
		overflow = numberOfBytes - charsLeft;
		numberOfBytes = charsLeft;
	}
	memcpy(destAddress, &sourceAddress[locationInBuffer], numberOfBytes);
	locationInBuffer += numberOfBytes;
	if (overflow > 0){ 
		memcpy(destAddress+numberOfBytes, &sourceAddress[0], overflow);
		locationInBuffer = overflow;
	}
}

GLbitfield ParserBuffer::fetch_GLbitfield_32()
{
	GLbitfield theValue;

	fetchBufferWrappedBytes(&theValue, sizeof(GLbitfield));
	return theValue;
}

GLclampfValue ParserBuffer::fetch_GLclampfValue_32()
{
	GLclampfValue theValue;
	fetchBufferWrappedBytes(&theValue, sizeof(GLclampfValue));
	return theValue;
}

GLclampxValue ParserBuffer::fetch_GLclampxValue_32()
{
	GLclampxValue theValue;
	fetchBufferWrappedBytes(&theValue, sizeof(GLclampxValue));
	return theValue;
}

GLenum ParserBuffer::fetch_GLenum() {
	GLenum theValue;
	fetchBufferWrappedBytes(&theValue, sizeof(GLenum));
	return theValue;
}
GLuint ParserBuffer::fetch_GLuint()
{
	GLuint theValue;
	fetchBufferWrappedBytes(&theValue, sizeof(GLuint));
	return theValue;
}

GLint ParserBuffer::fetch_GLint()
{
	GLint theValue;
	fetchBufferWrappedBytes(&theValue, sizeof(GLuint));
	return theValue;
}

GLsizei ParserBuffer::fetch_GLsizei()
{
	GLsizei theValue;
	fetchBufferWrappedBytes(&theValue, sizeof(GLsizei));
	return theValue;
}

void ParserBuffer::fetch_GLstring(int len, GLchar* buffer)
{
	fetchBufferWrappedBytes(buffer, len);
}
//TODO: are these correct?
GLsizeiptr ParserBuffer::fetch_GLsizeiptr()
{
	GLsizeiptr theValue;
	fetchBufferWrappedBytes(&theValue, sizeof(GLsizeiptr));
	return theValue;
}
