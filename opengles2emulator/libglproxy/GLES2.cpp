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
#include "GLES2.h"
#include "ParserCommon.h"
#include "AttribPointer.h"
#include "debug.h"
/*
 * The GL rendering functions.
 */


int programName;
extern AttribPointer **attribs;

extern void * indicesPtr;
extern GLuint* tokens;

AttribPointer* findAttribute(GLint index, bool createIfNecessary) {
	AttribPointer* ptr = NULL;
	for (int i=0;i<MAX_ATTRIBS;i++) 
	{
		if (attribs[i]!=NULL) {
			if (attribs[i]->index == index) {
				ptr = attribs[i];
				break;
			}	
		}
	}

	if (ptr == NULL && createIfNecessary) {
		ptr = new AttribPointer;
		ptr->index = index;
		for (int i=0;i<MAX_ATTRIBS;i++) 
		{
			if (attribs[i]==NULL) {
				attribs[i]=ptr;
				break;
			}
		}


	}
}

int getDataSize(GLenum type) 
{
	int len = 0;
	switch(type)
	{
		case GL_BYTE:
			len=sizeof(GLbyte);
			break;
		case GL_UNSIGNED_BYTE:
			len=sizeof(GLubyte);
			break;
		case GL_SHORT:
			len=sizeof(GLshort);
		case GL_UNSIGNED_SHORT:
			len=sizeof(GLushort);
			break;
		case GL_INT:
			len=sizeof(GLint);
		case GL_UNSIGNED_INT:
			len=sizeof(GLuint);
			break;
		case GL_FLOAT:
			len=sizeof(GLfloat);
			break;
		default:
			break;
	}
	return len;
}


void removeIncompatibleElement(char* shader, const char* ident) {
	char* replace;
	while (replace = strstr(shader, ident)) {
		memset(replace, ' ', strlen(ident));
	}
}

void removeIncompatibleElements(char* shader) {
	removeIncompatibleElement(shader, "precision mediump float;");
	removeIncompatibleElement(shader, "highp");
	removeIncompatibleElement(shader, "mediump");
	removeIncompatibleElement(shader, "lowp");
}

void showError() {
	GLint error = glGetError();
	if (error != GL_NO_ERROR) {
		DBG_PRINT("error %04x\n",error);	
	}

}


void setToken(GLuint token, GLuint realValue) {
	tokens[token] = realValue;
	if (token<0 || token >256) {
		DBG_PRINT("token error- out of range: %d\n", token);
		exit(0);
	}
	DBG_PRINT("token(%d) = %d\n",token, realValue);
}

GLuint getToken(GLuint token) {
	if (token<0 || token >256) {
		DBG_PRINT("token error- out of range: %d\n", token);
		exit(0);
	}
	GLuint val = tokens[token];
	DBG_PRINT("looked up token: %d val %d\n",token, val);
	return val; 
}


int parse_glCreateProgram(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	//		GLbitfield testField;

	DBG_PRINT("(%s)\n", __FUNCTION__);
	//		glClear(fetch_GLbitfield_32(theBufferAddress, locationInBuffer, bufferSize));
	GLuint token = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	int programName = glCreateProgram();
	showError();
	setToken(token, programName);

	return 0;
}

int parse_glCreateShader(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum shaderType = fetch_GLenum(theBufferAddress, locationInBuffer, bufferSize);
	GLuint token = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	int shader = glCreateShader(shaderType);
	showError();
	setToken(token, shader);
	return 0;
}

int parse_glShaderSource(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	int i;
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint shader = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	GLsizei count = fetch_GLsizei(theBufferAddress, locationInBuffer, bufferSize);		
	GLint* lengths = new GLint[count];
	GLchar** strings = new GLchar*[count];
	for (i=0;i<count;i++) {
		GLint len = fetch_GLint(theBufferAddress, locationInBuffer, bufferSize);
		lengths[i] = len;
		strings[i] = new GLchar[len];
		GLchar* str = fetch_GLstring(theBufferAddress, locationInBuffer, bufferSize, len);
		memcpy(strings[i],str,len);
		DBG_PRINT("%d %s\n", len, strings[i]);
		removeIncompatibleElements(strings[i]);
		DBG_PRINT("clean: %d %s\n", len, strings[i]);
	}
	glShaderSource(getToken(shader), count, (const GLchar**)strings, lengths);
	showError();

	for (i=0;i< count;i++) {
		delete[] strings[i];
	}
	delete[]strings;
	delete[]lengths;
	return 0;
}

int parse_glUseProgram(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	programName = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	GLuint realProgram = getToken(programName);
	glUseProgram(realProgram);
	showError();
	return 0;
}

int parse_glVertexAttribPointer(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	int i;
	GLuint index  = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	AttribPointer* ptr = findAttribute(index, true);
	ptr->size = fetch_GLint(theBufferAddress, locationInBuffer, bufferSize);		
	ptr->type = fetch_GLenum(theBufferAddress, locationInBuffer, bufferSize);	
	ptr->normalized = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);		
	ptr->stride = fetch_GLsizei(theBufferAddress, locationInBuffer, bufferSize);		
	ptr->length = fetch_GLint(theBufferAddress, locationInBuffer, bufferSize);		
	DBG_PRINT("pointer %08x\n", ptr->pointer);
	ptr->pointer = realloc(ptr->pointer, ptr->length);
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, (void*)ptr->pointer, ptr->length);
	float* data = (float*)ptr->pointer;
	for (i=0;i<ptr->length;i+=4) {
		DBG_PRINTV("%d %f\n", i>>2, *data++);
	}
	int realindex = getToken(index);
	glVertexAttribPointer(realindex, 
			ptr->size, 
			ptr->type, 
			ptr->normalized, 
			ptr->stride, 
			ptr->pointer);
	showError();
	// we only receive enabled arrays, so enable by default	
	glEnableVertexAttribArray(realindex);
	showError();
	return 0;
}
int parse_glUniform3fv(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	int i;
	GLuint index  = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	GLint count = fetch_GLint(theBufferAddress, locationInBuffer, bufferSize);		
	GLfloat* data = new GLfloat[count*3];
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, (void*)data, count*3*sizeof(GLfloat));
	for (i=0;i<count*3;i+=3) {
		DBG_PRINT("%d %f %f %f\n", i/3, data[i], data[i+1], data[i+2]);
	}
	int realindex = getToken(index);
	glUniform3fv(realindex, count, data);
	showError();
}
int parse_glUniformMatrix4fv(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	int i;
	GLuint index  = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	GLint count = fetch_GLint(theBufferAddress, locationInBuffer, bufferSize);		
	GLuint transVal  = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	GLfloat* data = new GLfloat[count*16];
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, (void*)data, count*16*sizeof(GLfloat));
	for (i=0;i<count*16;i+=16) {
		DBG_PRINT("%d %f %f %f %f\n", i>>4, data[i], data[i+1], data[i+2],data[i+3]);
		DBG_PRINT("%d %f %f %f %f\n", i>>4, data[i+4], data[i+5], data[i+6],data[i+7]);
		DBG_PRINT("%d %f %f %f %f\n", i>>4, data[i+8], data[i+9], data[i+10],data[i+12]);
		DBG_PRINT("%d %f %f %f %f\n", i>>4, data[i+12], data[i+13], data[i+14],data[i+15]);
	}
	int realindex = getToken(index);
	glUniformMatrix4fv(realindex, count,(GLboolean)transVal, data);
	showError();
}
// we only receive enabled arrays, so enable by default	
int parse_glEnableVertexAttribArray(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	DBG_PRINT("not used\n");
	return 0;
}
int parse_glLinkProgram(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	glLinkProgram(getToken(program));		
	showError();
	return 0;
}
int parse_glAttachShader(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	GLuint shader = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	glAttachShader(getToken(program), getToken(shader));
	showError();
	return 0;
}
int parse_glGetProgramInfoLog(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	return 0;
}
int parse_glDeleteProgram(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	glDeleteProgram(getToken(program));
	showError();
	return 0;
}
int parse_glGetAttribLocation(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);		
	GLuint token = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);		
	GLuint strlength = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);		
	GLchar* name = new GLchar[strlength];
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, name, strlength);
	DBG_PRINT("strlength %d str %s\n",strlength, name) ;
	DBG_PRINT("program %d token %d\n",program, token) ;
	GLuint realProgram = getToken(program);
	setToken(token, glGetAttribLocation(realProgram,name));
	showError();
	free(name);

	return 0;
}

int parse_glGetUniformLocation(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);		
	GLuint token = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);		
	GLuint strlength = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);		
	GLchar* name = new GLchar[strlength];
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, name, strlength);
	DBG_PRINT("strlength %d str %s\n",strlength, name) ;
	GLuint realProgram = getToken(program);
	setToken(token, glGetUniformLocation(realProgram,name));
	showError();
	free(name);

	return 0;
}

int parse_glCompileShader(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint shader;
	shader = fetch_GLuint(theBufferAddress, locationInBuffer, bufferSize);
	glCompileShader(getToken(shader));
	showError();
	return 0;
}
int parse_glDrawArrays(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum mode = fetch_GLenum(theBufferAddress, locationInBuffer, bufferSize);
	GLsizei count = fetch_GLsizei(theBufferAddress, locationInBuffer, bufferSize);
	DBG_PRINT("mode %d count %d\n",mode, count);
	glDrawArrays(mode, 0, count);	
	showError();
	return 0;
}

int parse_glDrawElements(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum mode = fetch_GLenum(theBufferAddress, locationInBuffer, bufferSize);
	GLsizei count = fetch_GLsizei(theBufferAddress, locationInBuffer, bufferSize);
	GLenum type = fetch_GLenum(theBufferAddress, locationInBuffer, bufferSize);
	int dataSize = getDataSize(type);
	int dataLength = dataSize*count;
	indicesPtr = realloc(indicesPtr, dataLength);	
	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, indicesPtr, dataLength);	
	DBG_PRINT("mode %d count %d type %d\n",mode, count, type);
	glDrawElements(mode, count, type, indicesPtr);	
	showError();
	return 0;
}

int parse_glEnable(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum cap = fetch_GLenum(theBufferAddress, locationInBuffer, bufferSize);
	glEnable(cap);
	showError();
	return 0;
}

int parse_glDisable(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum cap = fetch_GLenum(theBufferAddress, locationInBuffer, bufferSize);
	glDisable(cap);
	showError();
	return 0;
}
