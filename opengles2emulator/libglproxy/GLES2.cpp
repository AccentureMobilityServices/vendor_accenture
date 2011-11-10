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
#include "OpenGLES2Parser.h"
#include "ParserCommon.h"
#include "AttribPointer.h"
#include "debug.h"
/*
 * The GL rendering functions.
 */

AttribPointer* GLES2Parser::findAttribute(GLint index, bool createIfNecessary) {
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
	return ptr;
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

const char* shaderCompatibilityProg = "#define mediump\n"
										"#define highp\n"
										"#define lowp\n"
										"mat3 gles2emulator_matrix3(mat4 orig) {\n"
										"	mat3 val;\n"
										"	int i,j;\n"
										"	for (i=0;i<3;i++){\n"
										"		for (j=0;j<3;j++){\n"
										"			val[i][j] = orig[i][j];\n"
										"		}\n"
										"	}\n"
										"	return val;\n"
										"}\n"	
										"mat3 gles2emulator_matrix3(mat3 orig) {\n"
										"	mat3 val;\n"
										"	int i,j;\n"
										"	for (i=0;i<3;i++)\n"
										"		for (j=0;j<3;j++)\n"
										"			val[i][j] = orig[i][j];\n"
										"	return val;\n"
										"}\n"	
										"#define mat3(x) gles2emulator_matrix3(x)\n";
						
void removeIncompatibleLine(char* shader, const char* ident) {
	char* replace;
	char* endline;
	while (replace = strstr(shader, ident)) {
		endline = strstr(shader, ";");
		memset(replace, ' ', (endline-replace)+1);
	}
}

void removeIncompatibleElements(char* shader) {
	removeIncompatibleLine(shader, "precision");
}

void GLES2Parser::showError() {
	GLenum error = GL_NO_ERROR;
	if (theContext != NULL) {
		error = theContext->peekError();
	}
	if (error != GL_NO_ERROR) {
		DBG_PRINT("error %04x\n",error);
	}

}

void GLES2Parser::setReturnVal(int retVal)
{
	if (returnAddress!=NULL)
	{
		DBG_PRINT("Return value: %d\n",retVal);
		*(int*)returnAddress = retVal;
	}
}

int
GLES2Parser::parse_glClear()
{
GLbitfield testField;

		DBG_PRINT("(%s)\n", __FUNCTION__);
		glClear(buffer.fetch_GLbitfield_32());

		return 0;
}

int
GLES2Parser::parse_glClearColorf()
{
		DBG_PRINT("(%s)\n", __FUNCTION__);


		GLfloat r = buffer.fetch_GLclampfValue_32();
		GLfloat g = buffer.fetch_GLclampfValue_32();
		GLfloat b = buffer.fetch_GLclampfValue_32();
		GLfloat a = buffer.fetch_GLclampfValue_32();

		DBG_PRINT("color %f %f %f %f\n",r,g,b,a);
		glClearColor(r,g,b,a);
		return 0;
}

int
GLES2Parser::parse_glClearColorx()
{
		DBG_PRINT("(%s)\n", __FUNCTION__);
		GLfloat r = buffer.fetch_GLclampxValue_32();
		GLfloat g = buffer.fetch_GLclampxValue_32();
		GLfloat b = buffer.fetch_GLclampxValue_32();
		GLfloat a = buffer.fetch_GLclampxValue_32();

		DBG_PRINT("color %f %f %f %f\n",r,g,b,a);
		glClearColor(r,g,b,a);
		return 0;
}

int
GLES2Parser::parse_nullFunction()
{
		DBG_PRINT("(%s)\n", __FUNCTION__);
		return -1;
}


int GLES2Parser::parse_glCreateProgram()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	int programName = glCreateProgram();
	setReturnVal(programName);
	showError();

	return 0;
}

int GLES2Parser::parse_glCreateShader()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum shaderType = buffer.fetch_GLenum();
	int shader = glCreateShader(shaderType);
	setReturnVal(shader);
	showError();
	return 0;
}
int GLES2Parser::parse_glGetShaderiv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint shader = buffer.fetch_GLuint();
	GLuint param = buffer.fetch_GLenum();
	int val;
	glGetShaderiv(shader, param, &val);
	setReturnVal(val);
	showError();
	return 0;
}
int GLES2Parser::parse_glGetShaderInfoLog()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint shader = buffer.fetch_GLuint();
	GLsizei maxlen = buffer.fetch_GLsizei();
	glGetShaderInfoLog(shader, maxlen, (GLsizei*)returnAddress, ((GLchar*)returnAddress)+sizeof(GLsizei) );
	DBG_PRINT("Shader Log\n%s\n",(char*)((GLchar*)returnAddress)+sizeof(GLsizei));
	showError();
	return 0;
}
int GLES2Parser::parse_glGetProgramiv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = buffer.fetch_GLuint();
	GLenum param = buffer.fetch_GLenum();
	int val;
	glGetProgramiv(program, param, &val);
	setReturnVal(val);
	showError();
	return 0;
}

int GLES2Parser::parse_glShaderSource()
{
	int i;
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint shader = buffer.fetch_GLuint();
	GLsizei count = buffer.fetch_GLsizei();		
	GLint* lengths = new GLint[count+1];
	GLchar** strings = new GLchar*[count+1];
	GLint len = strlen(shaderCompatibilityProg); 
	// the first string passed to the pc GL implementation is a shader compatibility program
	strings[0] = new GLchar[len+1];
	strcpy(strings[0], shaderCompatibilityProg);
	lengths[0] = len;

	for (i=1;i<=count;i++) {
		len = buffer.fetch_GLint();
		lengths[i] = len;
		strings[i] = new GLchar[len+1];
		buffer.fetch_GLstring(len, strings[i]);
		strings[i][len] = '\0';
		
		removeIncompatibleElements(strings[i]); // we can't fix the standalone precision statement with the shaderCompat program
		DBG_PRINT("length: %d\n%s\n", len, strings[i]);
	}
	glShaderSource(shader, count+1, (const GLchar**)strings, lengths);
	showError();

	for (i=0;i< count;i++) {
		delete[] strings[i];
	}
	delete[]strings;
	delete[]lengths;
	return 0;
}

int GLES2Parser::parse_glUseProgram()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint programName = buffer.fetch_GLuint();
	DBG_PRINT("%d\n", programName);
	if (programName != 0)
		glUseProgram(programName);
	showError();
	return 0;
}

int GLES2Parser::parse_glVertexAttribPointer()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	int i;
	GLuint index  = buffer.fetch_GLuint();
	AttribPointer* ptr = findAttribute(index, true);
	ptr->size = buffer.fetch_GLint();
	ptr->type = buffer.fetch_GLenum();
	ptr->normalized = buffer.fetch_GLuint();
	ptr->stride = buffer.fetch_GLsizei();
	ptr->length = buffer.fetch_GLint();
	DBG_PRINT("pointer %08x\n",(unsigned int) ptr->pointer);
	ptr->pointer = realloc(ptr->pointer, ptr->length);
	DBG_PRINT("size %d length:%d\n",ptr->size, ptr->length);
	buffer.fetchBufferWrappedBytes((void*)ptr->pointer, ptr->length);
	float* data = (float*)ptr->pointer;
	for (i=0;i<ptr->length;i+=4) {
		DBG_PRINTV("%d %f\n", i>>2, *data++);
	}
	glVertexAttribPointer(index,
			ptr->size,
			ptr->type,
			ptr->normalized,
			ptr->stride,
			ptr->pointer);
	showError();
	// we only receive enabled arrays, so enable by default
	glEnableVertexAttribArray(index);
	showError();
	return 0;
}
int GLES2Parser::parse_glUniform3fv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	int i;
	GLuint index  = buffer.fetch_GLuint();
	GLint count = buffer.fetch_GLint();
	GLfloat* data = new GLfloat[count*3];
	buffer.fetchBufferWrappedBytes((void*)data, count*3*sizeof(GLfloat));
	for (i=0;i<count*3;i+=3) {
		DBG_PRINT("%d %f %f %f\n", i/3, data[i], data[i+1], data[i+2]);
	}
	glUniform3fv(index, count, data);
	showError();
}
int GLES2Parser::parse_glUniformMatrix4fv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	int i;
	GLuint index  = buffer.fetch_GLuint();
	GLint count = buffer.fetch_GLint();
	GLuint transVal  = buffer.fetch_GLuint();
	GLfloat* data = new GLfloat[count*16];
	buffer.fetchBufferWrappedBytes((void*)data, count*16*sizeof(GLfloat));
	for (i=0;i<count*16;i+=16) {
		DBG_PRINT("%d %f %f %f %f\n", i>>4, data[i], data[i+1], data[i+2],data[i+3]);
		DBG_PRINT("%d %f %f %f %f\n", i>>4, data[i+4], data[i+5], data[i+6],data[i+7]);
		DBG_PRINT("%d %f %f %f %f\n", i>>4, data[i+8], data[i+9], data[i+10],data[i+12]);
		DBG_PRINT("%d %f %f %f %f\n", i>>4, data[i+12], data[i+13], data[i+14],data[i+15]);
	}
	glUniformMatrix4fv(index, count,(GLboolean)transVal, data);
	showError();
}
// we only receive enabled arrays, so enable by default
int GLES2Parser::parse_glEnableVertexAttribArray()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint index = buffer.fetch_GLuint();
	glEnableVertexAttribArray(index);
	showError();
	return 0;
}
int GLES2Parser::parse_glLinkProgram()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = buffer.fetch_GLuint();
	glLinkProgram(program);
	showError();
	return 0;
}
int GLES2Parser::parse_glAttachShader()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = buffer.fetch_GLuint();
	GLuint shader = buffer.fetch_GLuint();
	DBG_PRINT("program %d shader %d\n", program, shader);
	glAttachShader(program, shader);
	showError();
	return 0;
}
int GLES2Parser::parse_glGetProgramInfoLog()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	return 0;
}
int GLES2Parser::parse_glDeleteProgram()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = buffer.fetch_GLuint();
	glDeleteProgram(program);
	showError();
	return 0;
}
int GLES2Parser::parse_glGetAttribLocation()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = buffer.fetch_GLuint();
	GLuint strlength = buffer.fetch_GLuint();
	GLchar* name = new GLchar[strlength];
	buffer.fetchBufferWrappedBytes(name, strlength);
	DBG_PRINT("strlength %d str %s\n",strlength, name) ;
	int attribLocation =glGetAttribLocation(program,name);
	setReturnVal(attribLocation);
	showError();
	delete[]name;

	return 0;
}

int GLES2Parser::parse_glGetUniformLocation()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = buffer.fetch_GLuint();
	GLuint strlength = buffer.fetch_GLuint();
	GLchar* name = new GLchar[strlength];
	buffer.fetchBufferWrappedBytes(name, strlength);
	DBG_PRINT("strlength %d str %s\n",strlength, name) ;
	int uniformLocation = glGetUniformLocation(program,name);
	setReturnVal(uniformLocation);
	showError();
	delete[]name;

	return 0;
}

int GLES2Parser::parse_glCompileShader()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint shader;
	shader = buffer.fetch_GLuint();
	glCompileShader(shader);
	showError();
	return 0;
}
int GLES2Parser::parse_glFrontFace()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum mode;
	mode = buffer.fetch_GLenum();
	glFrontFace(mode);
	showError();
	return 0;
}
int GLES2Parser::parse_glCullFace()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum mode;
	mode = buffer.fetch_GLenum();
	glCullFace(mode);
	showError();
	return 0;
}
int GLES2Parser::parse_glDrawArrays()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum mode = buffer.fetch_GLenum();
	GLsizei count = buffer.fetch_GLsizei();
	DBG_PRINT("mode %d count %d\n",mode, count);
	glDrawArrays(mode, 0, count);
	showError();
	return 0;
}

int GLES2Parser::parse_glDrawElements()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum mode = buffer.fetch_GLenum();
	GLsizei count = buffer.fetch_GLsizei();
	GLenum type = buffer.fetch_GLenum();
	int dataSize = getDataSize(type);
	int dataLength = dataSize*count;
	indicesPtr = realloc(indicesPtr, dataLength);
	buffer.fetchBufferWrappedBytes(indicesPtr, dataLength);
	DBG_PRINT("mode %d count %d type %d\n",mode, count, type);
	glDrawElements(mode, count, type, indicesPtr);
	showError();
	return 0;
}
int GLES2Parser::parse_glBlendColor()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLfloat r = buffer.fetch_GLclampfValue_32();
	GLfloat g = buffer.fetch_GLclampfValue_32();
	GLfloat b = buffer.fetch_GLclampfValue_32();
	GLfloat a = buffer.fetch_GLclampfValue_32();

	DBG_PRINT("color %f %f %f %f\n",r,g,b,a);
	glBlendColor(r,g,b,a);
	return 0;
}

int GLES2Parser::parse_glBlendEquation()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum mode = buffer.fetch_GLenum();
	DBG_PRINT("mode %d\n", mode);
	glBlendEquation(mode);
	return 0;
}

int GLES2Parser::parse_glBlendEquationSeparate()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum modeRGB = buffer.fetch_GLenum();
	GLenum modeAlpha = buffer.fetch_GLenum();
	DBG_PRINT("modeRGB %d modeAlpha %d\n", modeRGB, modeAlpha);
	//glBlendEquationSeparate(modeRGB, modeAlpha);
	return 0;
}

int GLES2Parser::parse_glBlendFunc()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum sfactor = buffer.fetch_GLenum();
	GLenum dfactor = buffer.fetch_GLenum();
	DBG_PRINT("sfactor %d, dfactor %d\n", sfactor, dfactor);
	glBlendFunc(sfactor, dfactor);
	return 0;
}

int GLES2Parser::parse_glBlendFuncSeparate()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum srcRGB = buffer.fetch_GLenum();
	GLenum dstRGB = buffer.fetch_GLenum();
	GLenum srcAlpha = buffer.fetch_GLenum();
	GLenum dstAlpha = buffer.fetch_GLenum();
	DBG_PRINT("srcRGB %d, dstRGB %d, srcAlpha %d, dstAlpha %d\n", srcRGB, dstRGB, srcAlpha, dstAlpha);
	glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
	return 0;
}

int GLES2Parser::parse_glBindAttribLocation()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = buffer.fetch_GLuint();
	GLuint index = buffer.fetch_GLuint();
	GLuint strlength = buffer.fetch_GLuint();
	GLchar* name = new GLchar[strlength];
	buffer.fetchBufferWrappedBytes(name, strlength);
	DBG_PRINT("strlength %d str %s\n",strlength, name) ;
	glBindAttribLocation(program, index, name);

	free(name);

	return 0;
}

int GLES2Parser::parse_glGenTextures()
{
	int i;
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLsizei n = buffer.fetch_GLsizei();
	GLuint* textures = (GLuint*)returnAddress;
	glGenTextures(n, textures);
	showError();

	return 0;

}

int GLES2Parser::parse_glActiveTexture()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum texture = buffer.fetch_GLenum();
	DBG_PRINT("texture %d\n", texture);
	glActiveTexture(texture);
	showError();
	return 0;
}

int GLES2Parser::parse_glBindTexture()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLuint texture = buffer.fetch_GLuint();
	DBG_PRINT("target %d, texture %d\n", target, texture);
	glBindTexture(target, texture);
	showError();
	return 0;
}

int GLES2Parser::parse_glFramebufferTexture2D()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLenum attachment = buffer.fetch_GLenum();
	GLenum textarget = buffer.fetch_GLenum();
	GLuint texture = buffer.fetch_GLuint();
	GLint level = buffer.fetch_GLint();

	DBG_PRINT("target %d, attachment %d, textarget %d, texture %d, level %d", target, attachment, textarget, texture, level);
	glFramebufferTexture2D(target, attachment, textarget, texture, level);
	showError();
	return 0;
}

int GLES2Parser::parse_glIsTexture()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint texture = buffer.fetch_GLuint();
	DBG_PRINT("texture %d", texture);
	glIsTexture(texture);
	return 0;
}

int GLES2Parser::parse_glDeleteTextures()
{
	int i;
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLsizei n = buffer.fetch_GLsizei();
	GLuint* textures = new GLuint[n];

	for (i=0;i<n;i++) {
		GLuint tex = buffer.fetch_GLint();
		textures[i] = tex;
		DBG_PRINT(" %d \n", tex);
	}
	glDeleteTextures(n, textures);
	showError();

	delete[]textures;
	return 0;
}

int GLES2Parser::parse_glBufferData()
{
	//TODO:
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLsizeiptr size = buffer.fetch_GLsizeiptr();
	GLenum usage = buffer.fetch_GLenum();
    return 0;
}


int GLES2Parser::parse_glBufferSubData()
{
	//TODO:
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();

	GLsizeiptr size = buffer.fetch_GLsizeiptr();

	return 0;
}

int GLES2Parser::parse_glCheckFramebufferStatus()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	DBG_PRINT("target %d\n", target);
	glCheckFramebufferStatus(target);
	showError();
	return 0;
}

int GLES2Parser::parse_glBindBuffer()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLuint buf = buffer.fetch_GLuint();
	DBG_PRINT("target %d, buffer %d\n", target, buf);
	glBindBuffer(target, buf);
	showError();
	return 0;
}

int	GLES2Parser::parse_glBindFramebuffer()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLuint framebuffer = buffer.fetch_GLuint();
	DBG_PRINT("target %d, framebuffer %d\n", target, framebuffer);
	glBindFramebuffer(target, framebuffer);
	showError();
	return 0;
}

int GLES2Parser::parse_glBindRenderbuffer()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLuint renderbuffer = buffer.fetch_GLuint();
	DBG_PRINT("target %d, renderbuffer %d\n", target, renderbuffer);
	glBindRenderbuffer(target, renderbuffer);
	showError();
	return 0;
}

int GLES2Parser::parse_glColorMask()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint red = buffer.fetch_GLuint();
	GLuint green = buffer.fetch_GLuint();
	GLuint blue = buffer.fetch_GLuint();
	GLuint alpha = buffer.fetch_GLuint();
	DBG_PRINT("red %d, green %d, blue %d, alpha %d\n", red, green, blue, alpha);
	glColorMask(red, green, blue, alpha);
	return 0;
}

int GLES2Parser::parse_glCompressedTexImage2D()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLint level = buffer.fetch_GLint();
	GLenum internalformat = buffer.fetch_GLenum();
	GLsizei width = buffer.fetch_GLsizei();
	GLsizei height = buffer.fetch_GLsizei();
	GLint border = buffer.fetch_GLint();
	GLsizei imageSize = buffer.fetch_GLsizei();
	//TODO: const GLvoid* data
	//DBG_PRINT("");
	//glCompressedTexImage2();
	return 0;
}

int GLES2Parser::parse_glCompressedTexSubImage2D()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLint level = buffer.fetch_GLint();
	GLint xoffset = buffer.fetch_GLint();
	GLint yoffset = buffer.fetch_GLint();
	GLsizei width = buffer.fetch_GLsizei();
	GLsizei height = buffer.fetch_GLsizei();
	GLenum format = buffer.fetch_GLenum();
	GLsizei imageSize = buffer.fetch_GLsizei();
	//TODO: const GLvoid* data
	//DBG_PRINT("");
	//glCompressedTexSubImage2D();
	return 0;
}

int GLES2Parser::parse_glCopyTexImage2D()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLint level = buffer.fetch_GLint();
	GLenum internalformat = buffer.fetch_GLenum();
	GLint x = buffer.fetch_GLint();
	GLint y = buffer.fetch_GLint();
	GLsizei width = buffer.fetch_GLsizei();
	GLsizei height = buffer.fetch_GLsizei();
	GLint border = buffer.fetch_GLint();
	DBG_PRINT("target %d, level %d, internalformat %d, x %d, y %d, width %d, height %d, border %d", target, level, internalformat, x, y, width, height, border);
	glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
	return 0;
}

int GLES2Parser::parse_glCopyTexSubImage2D()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLint level = buffer.fetch_GLint();
	GLint xoffset = buffer.fetch_GLint();
	GLint yoffset = buffer.fetch_GLint();
	GLint x = buffer.fetch_GLint();
	GLint y = buffer.fetch_GLint();
	GLsizei width = buffer.fetch_GLsizei();
	GLsizei height = buffer.fetch_GLsizei();
	DBG_PRINT("target %d, level %d, xoffset %d, yoffset %d, x %d, y %d, width %d, height %d\n", target, level, xoffset, yoffset, x, y, width, height);
	glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	return 0;
}

int GLES2Parser::parse_glDeleteBuffers()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);

	GLsizei n = buffer.fetch_GLsizei();
	GLuint* lengths = new GLuint[n];

	int i;
	for (i=0;i<n;i++) {
		GLuint len = buffer.fetch_GLuint();
		lengths[i] = len;
		DBG_PRINT("%d \n", len);
	}
	glDeleteBuffers(n, lengths);
	delete[]lengths;
	return 0;
}

int	GLES2Parser::parse_glDeleteFramebuffers()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);

	GLsizei n = buffer.fetch_GLsizei();
	GLuint* lengths = new GLuint[n];

	int i;
	for (i=0;i<n;i++) {
		GLuint len = buffer.fetch_GLuint();
		lengths[i] = len;
		DBG_PRINT("%d \n", len);
	}
	glDeleteFramebuffers(n, lengths);
	delete[]lengths;
	return 0;
}

int GLES2Parser::parse_glDeleteRenderbuffers()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);

	GLsizei n = buffer.fetch_GLsizei();
	GLuint* lengths = new GLuint[n];

	int i;
	for (i=0;i<n;i++) {
		GLuint len = buffer.fetch_GLuint();
		lengths[i] = len;
		DBG_PRINT("%d \n", len);
	}
	glDeleteRenderbuffers(n, lengths);
	delete[]lengths;
	return 0;
}

int GLES2Parser::parse_glDepthFunc()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum func = buffer.fetch_GLenum();
	DBG_PRINT("func %d\n", func);
	glDepthFunc(func);
	return 0;
}

int GLES2Parser::parse_glDepthMask()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO:
	return 0;
}

int GLES2Parser::parse_glDepthRangef()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO:
	return 0;
}

int GLES2Parser::parse_glDetachShader()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = buffer.fetch_GLuint();
	GLuint shader = buffer.fetch_GLuint();
	DBG_PRINT("program %d, shader %d\n", program, shader);
	glDetachShader(program, shader);
	return 0;
}

int GLES2Parser::parse_glDisableVertexAttribArray()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint index = buffer.fetch_GLuint();
	DBG_PRINT("index %d", index);
	glDisableVertexAttribArray(index);
	return 0;
}

int GLES2Parser::parse_glFramebufferRenderbuffer()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLenum attachment = buffer.fetch_GLenum();
	GLenum renderbuffertarget = buffer.fetch_GLenum();
	GLuint renderbuffer = buffer.fetch_GLuint();
	DBG_PRINT("target %d, attachment %d, renderbuffertarget %d, renderbuffer %d\n", target, attachment, renderbuffertarget, renderbuffer);
	glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
	return 0;
}

int GLES2Parser::parse_glGenBuffers()
{
	int i;
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLsizei n = buffer.fetch_GLsizei();
	GLuint* buffers = (GLuint*)returnAddress;
	glGenBuffers(n, buffers);
	showError();
	return 0;
}

int GLES2Parser::parse_glGenerateMipmap()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	DBG_PRINT("target %d", target);
	glGenerateMipmap(target);
	showError();
	return 0;
}

int GLES2Parser::parse_glGenFramebuffers()
{
	int i;
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLsizei n = buffer.fetch_GLsizei();
	GLuint* buffers = (GLuint*)returnAddress;
	glGenFramebuffers(n, buffers);
	showError();
	return 0;
}

int GLES2Parser::parse_glGenRenderbuffers()
{
	int i;
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLsizei n = buffer.fetch_GLsizei();
	GLuint* buffers = (GLuint*)returnAddress;
	glGenRenderbuffers(n, buffers);
	showError();
	return 0;
}

int GLES2Parser::parse_glGetActiveAttrib()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLsizei* length, GLint* size, GLenum* type, GLchar* name
	GLuint program = buffer.fetch_GLuint();
	GLuint index = buffer.fetch_GLuint();
	GLsizei bufsize = buffer.fetch_GLsizei();

	return 0;
}

int GLES2Parser::parse_glGetActiveUniform()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name
	return 0;
}

int GLES2Parser::parse_glGetAttachedShaders()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders
	return 0;
}

int GLES2Parser::parse_glGetBooleanv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLenum pname, GLboolean* params
	return 0;
}

int GLES2Parser::parse_glGetBufferParameteriv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLenum target, GLenum pname, GLint* params
	return 0;
}

int GLES2Parser::parse_glGetFloatv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLenum pname, GLfloat* params
	return 0;
}

int GLES2Parser::parse_glGetFramebufferAttachmentParameteriv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLenum target, GLenum attachment, GLenum pname, GLint* params
	return 0;
}

int	GLES2Parser::parse_glGetIntegerv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO:GLenum pname, GLint* params
	return 0;
}

int GLES2Parser::parse_glGetRenderbufferParameteriv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLenum target, GLenum pname, GLint* params
	return 0;
}

int GLES2Parser::parse_glGetShaderPrecisionFormat()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision
	return 0;
}

int GLES2Parser::parse_glGetShaderSource()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source
	return 0;
}

int GLES2Parser::parse_glGetTexParameterfv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLenum target, GLenum pname, GLfloat* params
	return 0;
}

int GLES2Parser::parse_glGetTexParameteriv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
    //TODO: GLenum target, GLenum pname, GLint* params
	return 0;
}

int GLES2Parser::parse_glGetUniformfv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLuint program, GLint location, GLfloat* params
	return 0;
}

int GLES2Parser::parse_glGetUniformiv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLuint program, GLint location, GLint* params
	return 0;
}

int GLES2Parser::parse_glGetVertexAttribfv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLuint index, GLenum pname, GLfloat* params
	return 0;
}

int GLES2Parser::parse_glGetVertexAttribiv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLuint index, GLenum pname, GLint* params
	return 0;
}

int GLES2Parser::parse_glGetVertexAttribPointerv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO: GLuint index, GLenum pname, GLvoid** pointer
	return 0;
}

int GLES2Parser::parse_glIsBuffer()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint buf = buffer.fetch_GLuint();
	DBG_PRINT("buffer %d\n", buf);
	GLboolean isBuff = glIsBuffer(buf);
	return isBuff;
}

int GLES2Parser::parse_glIsEnabled()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum cap = buffer.fetch_GLenum();
	GLboolean isEnable = glIsEnabled(cap);
	return isEnable;
}

int GLES2Parser::parse_glIsFramebuffer()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint framebuffer = buffer.fetch_GLuint();
	GLboolean isFramebuff = glIsFramebuffer(framebuffer);
	return isFramebuff;
}

int GLES2Parser::parse_glIsProgram()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = buffer.fetch_GLuint();
	GLboolean isProg = glIsProgram(program);
	return isProg;
}

int GLES2Parser::parse_glIsRenderbuffer()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint renderbuffer = buffer.fetch_GLuint();
	GLboolean isRendbuff = glIsRenderbuffer(renderbuffer);
	return isRendbuff;
}

int GLES2Parser::parse_glIsShader()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint shader = buffer.fetch_GLuint();
	GLboolean isShade = glIsShader(shader);
	return isShade;
}

int GLES2Parser::parse_glLineWidth()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLfloat width = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("width %f\n", width);
	glLineWidth(width);
	return 0;
}

int GLES2Parser::parse_glPixelStorei()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum pname = buffer.fetch_GLenum();
	GLint param = buffer.fetch_GLint();
	DBG_PRINT("pname %d, param %d\n", pname, param);
	return 0;
}

int GLES2Parser::parse_glPolygonOffset()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLfloat factor = buffer.fetch_GLclampfValue_32();
	GLfloat units = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("factor %f, units %f\n", factor, units);
	glPolygonOffset(factor, units);
	return 0;
}

int GLES2Parser::parse_glReadPixels()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO:
	return 0;
}

int GLES2Parser::parse_glReleaseShaderCompiler()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO:
	return 0;
}

int GLES2Parser::parse_glRenderbufferStorage()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLenum internalformat = buffer.fetch_GLenum();
	GLsizei width = buffer.fetch_GLsizei();
	GLsizei height = buffer.fetch_GLsizei();
	DBG_PRINT("target %d, internalformat %04x, width %d, height %d\n", target, internalformat, width, height);
	glRenderbufferStorage(target, internalformat, width, height);
	showError();
	return 0;
}

int GLES2Parser::parse_glScissor()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint x = buffer.fetch_GLint();
	GLint y = buffer.fetch_GLint();
	GLsizei width = buffer.fetch_GLsizei();
	GLsizei height = buffer.fetch_GLsizei();
	DBG_PRINT("x %d, y %d, width %d, height %d\n", x, y, width, height);
	glScissor(x, y, width, height);
	return 0;
}

int GLES2Parser::parse_glShaderBinary()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	//TODO:
	return 0;
}

int GLES2Parser::parse_glStencilFuncSeparate()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum face = buffer.fetch_GLenum();
	GLenum func = buffer.fetch_GLenum();
	GLint ref = buffer.fetch_GLint();
	GLuint mask = buffer.fetch_GLuint();
	DBG_PRINT("face %d, func %d, ref %d, mask %d\n", face, func, ref, mask);
	glStencilFuncSeparate(face, func, ref, mask);
	return 0;
}

int GLES2Parser::parse_glStencilMask()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum mask = buffer.fetch_GLenum();
	DBG_PRINT("mask %d\n", mask);
	glStencilMask(mask);
	return 0;
}

int GLES2Parser::parse_glStencilMaskSeparate()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum face = buffer.fetch_GLenum();
	GLuint mask = buffer.fetch_GLuint();
	DBG_PRINT("face %d, mask %d\n", face, mask);
	glStencilMaskSeparate(face, mask);
	return 0;
}

int GLES2Parser::parse_glStencilOp()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum fail = buffer.fetch_GLenum();
	GLenum zfail = buffer.fetch_GLenum();
	GLenum zpass = buffer.fetch_GLenum();
	DBG_PRINT("fail %d, zfail %d, zpass %d\n", fail, zfail, zpass);
	glStencilOp(fail, zfail, zpass);
	return 0;
}

int GLES2Parser::parse_glStencilOpSeparate()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum face = buffer.fetch_GLenum();
	GLenum fail = buffer.fetch_GLenum();
	GLenum zfail = buffer.fetch_GLenum();
	GLenum zpass = buffer.fetch_GLenum();
	DBG_PRINT("face %d, fail %d, zfail %d, zpass %d\n", face, fail, zfail, zpass);
	glStencilOpSeparate(face, fail, zfail, zpass);
	return 0;
}
static int texnum = 1;
void writeTexture(GLsizei width, GLsizei height, GLenum type, unsigned char* texture) {
	char fileName[256];
	sprintf(fileName, "texture%d.pbm",texnum++);
	FILE* fd = fopen(fileName,"w");

	char buf[32];
	if (type == GL_RGBA) 
	{
		sprintf(buf, "P3\n%d %d\n255\n",width,height);
		fwrite(buf, 1, strlen(buf),fd);	
		for (int i=0;i<height;i++) {

			for (int j=0;j<width;j++) {
				sprintf(buf,"%d %d %d ",*texture++, *texture++, *texture++);
				texture++; // dump alpha - we don't care
				fwrite(buf, 1, strlen(buf),fd);	
			}
			sprintf(buf, "\n");
			fwrite(buf, 1, strlen(buf), fd);
		}
	} else {
		sprintf(buf, "P3\n%d %d\n255\n",width,height);
		fwrite(buf, 1, strlen(buf),fd);	
		for (int i=0;i<height;i++) {
			for (int j=0;j<width;j++) {
				unsigned char pixel = *texture++;
				sprintf(buf,"%d %d %d ",pixel,pixel,pixel);
				fwrite(buf, 1, strlen(buf),fd);	
			}
			sprintf(buf, "\n");
			fwrite(buf, 1, strlen(buf), fd);	
		}
	}
	fclose(fd);
}

int GLES2Parser::parse_glTexImage2D()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLint level = buffer.fetch_GLint();
	GLenum internalFormat = buffer.fetch_GLenum();
	GLsizei width = buffer.fetch_GLsizei();
	GLsizei height = buffer.fetch_GLsizei();
	GLint border = buffer.fetch_GLint();
	GLenum format = buffer.fetch_GLenum();
	GLenum type = buffer.fetch_GLenum();
	GLint size = buffer.fetch_GLint();
	DBG_PRINT("texture %04x %04x %04x w:%d h:%d border:%d, format:%04x, type:%04x, size: %d\n",target, level, internalFormat, width, height, border, format, type, size);

	char* texture = NULL;
	if (size!=0) {
		texture = new char[size];
		buffer.fetchBufferWrappedBytes(texture, size);
		//writeTexture(width, height, format, (unsigned char*)texture);
	}
	DBG_PRINT("texture pointer 0x%08x\n",(unsigned int) texture);
	glTexImage2D(target,level,internalFormat, width, height, border, format, type, texture);
	showError();
	//delete[]texture;
	return 0;
}

int GLES2Parser::parse_glTexParameterf()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLenum pname = buffer.fetch_GLenum();
	GLfloat param = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("target %d, pname %d, param %f\n", target, pname, param);
	glTexParameterf(target, pname, param);
	return 0;
}

int GLES2Parser::parse_glTexParameterfv()
{
	int i;
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLenum pname = buffer.fetch_GLenum();
	GLint count = buffer.fetch_GLint();
	GLfloat* params = new GLfloat[count];
	buffer.fetchBufferWrappedBytes((void*)params, count*sizeof(GLfloat));
	for (i=0;i<count;i++) {
		DBG_PRINT("target %d, pname %d, params %f\n", target, pname, params[i]);
	}
	glTexParameterfv(target, pname, params);
	return 0;
}

int GLES2Parser::parse_glTexParameteri()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLenum pname = buffer.fetch_GLenum();
	GLint param = buffer.fetch_GLint();
	DBG_PRINT("target %d, pname %d, param %d\n", target, pname, param);
	glTexParameteri(target, pname, param);
	return 0;
}

int GLES2Parser::parse_glTexParameteriv()
{
	int i;
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLenum pname = buffer.fetch_GLenum();
	GLsizei count = buffer.fetch_GLsizei();
	GLint* params = new GLint[count];
	buffer.fetchBufferWrappedBytes((void*)params, count*sizeof(GLint));
	for (i=0;i<count;i++) {
		DBG_PRINT("target %d, pname %d, params %d\n", target, pname, params[i]);
	}
	glTexParameteriv(target, pname, params);
	return 0;
}

int GLES2Parser::parse_glTexSubImage2D()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum target = buffer.fetch_GLenum();
	GLint level = buffer.fetch_GLint();
	GLint xoffset = buffer.fetch_GLint();
	GLint yoffset = buffer.fetch_GLint();
	GLsizei width = buffer.fetch_GLsizei();
	GLsizei height = buffer.fetch_GLsizei();
	GLenum format = buffer.fetch_GLenum();
	GLenum type = buffer.fetch_GLenum();
	GLint size = buffer.fetch_GLint();
	DBG_PRINT("texture %04x %04x %04x %04x w:%d h:%d, format:%04x, type:%04x, size: %d\n",target, level, xoffset, yoffset, width, height, format, type, size);

	char* texture = NULL;
	if (size!=0) {
		char* texture = new char[size];
		buffer.fetchBufferWrappedBytes(texture, size);
		writeTexture(width, height, format, (unsigned char*)texture);
	}
	glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, texture);
	showError();
	//delete[]texture;
	return 0;
}

int GLES2Parser::parse_glUniform1f()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLfloat x = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("location %d, x %f\n", location, x);
	if (location>=0)
		glUniform1f(location, x);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform1fv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLint count = buffer.fetch_GLint();
	GLfloat* v = new GLfloat[count];
	buffer.fetchBufferWrappedBytes((void*)v, count*sizeof(GLfloat));
	//DBG_PRINT("location %d, count %d, v %f", location, count, v);
	glUniform1fv(location, count, v);
	return 0;
}

int GLES2Parser::parse_glUniform1i()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLint x = buffer.fetch_GLint();
	DBG_PRINT("location %d, x %d\n", location, x);
	glUniform1i(location, x);
	return 0;
}

int GLES2Parser::parse_glUniform1iv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLint count = buffer.fetch_GLint();
	GLint* data = new GLint[count];
	buffer.fetchBufferWrappedBytes((void*)data, count*sizeof(GLint));
	//DBG_PRINT("location %d, count %d, data %f", location, count, data);
	glUniform1iv(location, count, data);
	return 0;
}

int GLES2Parser::parse_glUniform2f()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLfloat x = buffer.fetch_GLclampfValue_32();
	GLfloat y = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("location %d, x %f, y %f\n", location, x, y);
	glUniform2f(location, x, y);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform2fv()
{
	int i;
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLint count = buffer.fetch_GLint();
	GLfloat* data = new GLfloat[count*2];
	buffer.fetchBufferWrappedBytes((void*)data, count*sizeof(GLfloat));
	for (i=0;i<count*2;i+=2) {
		DBG_PRINT("%d %f %f\n", i/2, data[i], data[i+1]);
	}
	glUniform2fv(location, count, data);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform2i()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLint x = buffer.fetch_GLint();
	GLint y = buffer.fetch_GLint();
	DBG_PRINT("location %d, x %d, y %d\n", location, x, y);
	glUniform2i(location, x, y);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform2iv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	int i;
	GLuint location  = buffer.fetch_GLuint();
	GLint count = buffer.fetch_GLint();
	GLint* data = new GLint[count*2];
	buffer.fetchBufferWrappedBytes((void*)data, count*2*sizeof(GLint));
	for (i=0;i<count*2;i+=2) {
		DBG_PRINT("%d %d %d \n", i/2, data[i], data[i+1]);
	}
	glUniform2iv(location, count, data);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform3f()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLfloat x = buffer.fetch_GLclampfValue_32();
	GLfloat y = buffer.fetch_GLclampfValue_32();
	GLfloat z = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("location %d, x %f, y %f, z %f\n", location, x, y, z);
	glUniform3f(location, x, y, z);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform3i()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLint x = buffer.fetch_GLint();
	GLint y = buffer.fetch_GLint();
	GLint z = buffer.fetch_GLint();
	DBG_PRINT("location %d, x %d, y %d, z %d\n", location, x, y, z);
	glUniform3i(location, x, y, z);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform3iv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	int i;
	GLuint location  = buffer.fetch_GLuint();
	GLint count = buffer.fetch_GLint();
	GLint* data = new GLint[count*3];
	buffer.fetchBufferWrappedBytes((void*)data, count*3*sizeof(GLint));
	for (i=0;i<count*3;i+=3) {
		DBG_PRINT("%d %d %d %d\n", i/3, data[i], data[i+1], data[i+2]);
	}
	glUniform3iv(location, count, data);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform4f()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLfloat x = buffer.fetch_GLclampfValue_32();
	GLfloat y = buffer.fetch_GLclampfValue_32();
	GLfloat z = buffer.fetch_GLclampfValue_32();
	GLfloat w = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("location %d, x %f ,y %f ,z %f ,w %f \n", location, x, y, z, w);
	glUniform4f(location, x, y, z, w);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform4fv()
{

	DBG_PRINT("(%s)\n", __FUNCTION__);
	int i;
	GLuint location  = buffer.fetch_GLuint();
	GLint count = buffer.fetch_GLint();
	GLfloat* data = new GLfloat[count*4];
	buffer.fetchBufferWrappedBytes((void*)data, count*4*sizeof(GLfloat));
	for (i=0;i<count*4;i+=4) {
		DBG_PRINT("%d %f %f %f %f\n", i/4, data[i], data[i+1], data[i+2], data[i+3]);
	}
	glUniform4fv(location, count, data);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform4i()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLint location = buffer.fetch_GLint();
	GLint x = buffer.fetch_GLint();
	GLint y = buffer.fetch_GLint();
	GLint z = buffer.fetch_GLint();
	GLint w = buffer.fetch_GLint();
	DBG_PRINT("location %d, x %d, y %d, z %d, w %d\n", location, x, y, z, w);
	glUniform4i(location, x, y, z, w);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniform4iv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	int i;
	GLuint location  = buffer.fetch_GLuint();
	GLint count = buffer.fetch_GLint();
	GLint* data = new GLint[count*4];
	buffer.fetchBufferWrappedBytes((void*)data, count*4*sizeof(GLint));
	for (i=0;i<count*4;i+=4) {
		DBG_PRINT("%d %d %d %d %d\n", i/4, data[i], data[i+1], data[i+2], data[i+3]);
	}
	glUniform4iv(location, count, data);
	showError();
	return 0;
}

int GLES2Parser::parse_glUniformMatrix2fv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);

	return 0;
}

int GLES2Parser::parse_glUniformMatrix3fv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);

	return 0;
}

int GLES2Parser::parse_glValidateProgram()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint program = buffer.fetch_GLuint();
	DBG_PRINT("program %d\n", program);
	glValidateProgram(program);
	return 0;
}

int GLES2Parser::parse_glVertexAttrib1f()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint index = buffer.fetch_GLuint();
	GLfloat x = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("index %d, x %f\n", index, x);
	glVertexAttrib1f(index, x);
	return 0;
}

int GLES2Parser::parse_glVertexAttrib1fv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint index = buffer.fetch_GLuint();
	GLint count = buffer.fetch_GLint();
	GLfloat* values = new GLfloat[count];
	buffer.fetchBufferWrappedBytes((void*)values, count*sizeof(GLfloat));
	//DBG_PRINT("index %d, values %f\n", index, values);
	glVertexAttrib1fv(index, values);
	return 0;
}

int GLES2Parser::parse_glVertexAttrib2f()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint index = buffer.fetch_GLuint();
	GLfloat x = buffer.fetch_GLclampfValue_32();
	GLfloat y = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("index %d, x %f, y %f\n", index, x, y);
	glVertexAttrib2f(index, x, y);
	return 0;
}

int GLES2Parser::parse_glVertexAttrib2fv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);

	return 0;
}

int GLES2Parser::parse_glVertexAttrib3f()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint index = buffer.fetch_GLuint();
	GLfloat x = buffer.fetch_GLclampfValue_32();
	GLfloat y = buffer.fetch_GLclampfValue_32();
	GLfloat z = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("index %d, x %f, y %f, z %f\n", index, x, y, z);
	glVertexAttrib3f(index, x, y, z);
	return 0;
}

int GLES2Parser::parse_glVertexAttrib3fv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);

	return 0;
}

int GLES2Parser::parse_glVertexAttrib4f()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint index = buffer.fetch_GLuint();
	GLfloat x = buffer.fetch_GLclampfValue_32();
	GLfloat y = buffer.fetch_GLclampfValue_32();
	GLfloat z = buffer.fetch_GLclampfValue_32();
	GLfloat w = buffer.fetch_GLclampfValue_32();
	DBG_PRINT("index %d, x %f, y %f, z %f, w %f\n", index, x, y, z, w);
	glVertexAttrib4f(index, x, y, z, w);
	return 0;
}

int GLES2Parser::parse_glVertexAttrib4fv()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);

	return 0;
}

int GLES2Parser::parse_glEnable()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum cap = buffer.fetch_GLenum();
	glEnable(cap);
	showError();
	return 0;
}

int GLES2Parser::parse_glDisable()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum cap = buffer.fetch_GLenum();
	glDisable(cap);
	showError();
	return 0;
}

int GLES2Parser::parse_glViewport()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLuint x;
	GLuint y;
	GLsizei w;
	GLsizei h;
	x = buffer.fetch_GLuint();
	y = buffer.fetch_GLuint();
	w = buffer.fetch_GLuint();
	h = buffer.fetch_GLuint();

	DBG_PRINT("viewport %d %d %d %d\n",x,y,w,h);

	glViewport(x,y,w,h);
	showError();
	return 0;
}

int GLES2Parser::parse_glGetError()
{
	DBG_PRINT("(%s)\n", __FUNCTION__);
	GLenum errval = GL_NO_ERROR;
	if (theContext != NULL) {
		errval = theContext->getError();
	}
	//errval = GL_NO_ERROR;
	setReturnVal(errval);
}
