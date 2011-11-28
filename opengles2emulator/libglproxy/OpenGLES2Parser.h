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
#ifndef OPENGLES2PARSER_H_
#define OPENGLES2PARSER_H_

#include <stdio.h>
#include "glheaders.h" 
#include "AttribPointer.h"
#include "ParserCommon.h"
#include "glproxy_context.h"

#define NUMBER_OF_GLES2_POINTERS 256

class SharedMemory;

class GLES2Parser {
public:
	GLES2Parser(ParserBuffer& buffer);

	void parseGLES2Command(int payloadSize, void* returnAddress);
	void setContext(GLproxyContext* context);
	void setSharedMemory(SharedMemory* sharedMemory){
		androidSharedMemory = sharedMemory;
	}
private:
	int initOpenGLES2Parser();
	
	void parse_nullFunction();
	void parse_glClear();
	void parse_glClearColorf();
	void parse_glClearColorx();
	void parse_glCreateProgram();
	void parse_glCreateShader();
	void parse_glShaderSource();
	void parse_glUseProgram();
	void parse_glVertexAttribPointer();
	void parse_glEnableVertexAttribArray();
	void parse_glAttachShader();
	void parse_glLinkProgram();
	void parse_glGetProgramInfoLog();
	void parse_glDeleteProgram();
	void parse_glDeleteShader();
	void parse_glGetAttribLocation();
	void parse_glCompileShader();
	void parse_glDrawArrays();
	void parse_glGetUniformLocation();
	void parse_glUniformMatrix4fv();
	void parse_glUniform3fv();
	void parse_glDisable();
	void parse_glEnable();
	void parse_glDrawElements();

	void parse_glBlendColor();
	void parse_glBlendEquation();
	void parse_glBlendEquationSeparate();
	void parse_glBlendFunc();
	void parse_glBlendFuncSeparate();
	void parse_glBindAttribLocation();
	void parse_glGenTextures();
	void parse_glActiveTexture();
	void parse_glBindTexture();
	void parse_glFramebufferTexture2D();
	void parse_glIsTexture();
	void parse_glDeleteTextures();

	void parse_glBufferData();
	void parse_glBufferSubData();
	void parse_glCheckFramebufferStatus();
	void parse_glBindBuffer();
	void parse_glBindFramebuffer();
	void parse_glBindRenderbuffer();
	void parse_glColorMask();
	void parse_glCompressedTexImage2D();
	void parse_glCompressedTexSubImage2D();
	void parse_glCopyTexImage2D();
	void parse_glCopyTexSubImage2D();
	void parse_glDeleteBuffers();
	void parse_glDeleteFramebuffers();
	void parse_glDeleteRenderbuffers();
	void parse_glDepthFunc();
	void parse_glDepthMask();
	void parse_glDepthRangef();
	void parse_glDetachShader();
	void parse_glDisableVertexAttribArray();
	void parse_glFramebufferRenderbuffer();
	void parse_glGenBuffers();
	void parse_glGenerateMipmap();
	void parse_glGenFramebuffers();
	void parse_glGenRenderbuffers();
	void parse_glGetActiveAttrib();
	void parse_glGetActiveUniform();
	void parse_glGetAttachedShaders();
	void parse_glGetBooleanv();
	void parse_glGetBufferParameteriv();
	void parse_glGetFloatv();
	void parse_glGetFramebufferAttachmentParameteriv();
	void parse_glGetIntegerv();
	void parse_glGetRenderbufferParameteriv();
	void parse_glGetShaderPrecisionFormat();
	void parse_glGetShaderSource();
	void parse_glGetTexParameterfv();
	void parse_glGetTexParameteriv();
	void parse_glGetUniformfv();
	void parse_glGetUniformiv();
	void parse_glGetVertexAttribfv();
	void parse_glGetVertexAttribiv();
	void parse_glGetVertexAttribPointerv();
	void parse_glIsBuffer();
	void parse_glIsEnabled();
	void parse_glIsFramebuffer();
	void parse_glIsProgram();
	void parse_glIsRenderbuffer();
	void parse_glIsShader();
	void parse_glLineWidth();
	void parse_glPixelStorei();
	void parse_glPolygonOffset();
	void parse_glReadPixels();
	void parse_glReleaseShaderCompiler();
	void parse_glRenderbufferStorage();
	void parse_glScissor();
	void parse_glShaderBinary();
	void parse_glStencilFuncSeparate();
	void parse_glStencilMask();
	void parse_glStencilMaskSeparate();
	void parse_glStencilOp();
	void parse_glStencilOpSeparate();
	void parse_glTexImage2D();
	void parse_glTexParameterf();
	void parse_glTexParameterfv();
	void parse_glTexParameterx();
	void parse_glTexParameteri();
	void parse_glTexParameteriv();
	void parse_glTexSubImage2D();
	void parse_glUniform1f();
	void parse_glUniform1fv();
	void parse_glUniform1i();
	void parse_glUniform1iv();
	void parse_glUniform2f();
	void parse_glUniform2fv();
	void parse_glUniform2i();
	void parse_glUniform2iv();
	void parse_glUniform3f();
	void parse_glUniform3i();
	void parse_glUniform3iv();
	void parse_glUniform4f();
	void parse_glUniform4fv();
	void parse_glUniform4i();
	void parse_glUniform4iv();
	void parse_glUniformMatrix2fv();
	void parse_glUniformMatrix3fv();
	void parse_glValidateProgram();
	void parse_glVertexAttrib1f();
	void parse_glVertexAttrib1fv();
	void parse_glVertexAttrib2f();
	void parse_glVertexAttrib2fv();
	void parse_glVertexAttrib3f();
	void parse_glVertexAttrib3fv();
	void parse_glVertexAttrib4f();
	void parse_glVertexAttrib4fv();

	void parse_glViewport();
	void parse_glGetError();
	void parse_glGetShaderiv();
	void parse_glGetShaderInfoLog();
	void parse_glGetProgramiv();
	void parse_glFrontFace();
	void parse_glCullFace();
	void parse_glFlush();
	void parse_glDrawTexiOES();
	
	void parse_sendVertexAttribPointer();
	void parse_glNativeImage2D();

private:
	AttribPointer* findAttribute(GLint index, bool createIfNecessary);
	void setReturnVal(int val);
	void showError();

	typedef void (GLES2Parser::*multifp)(); 
	multifp parserGLES2FunctionPointers[NUMBER_OF_GLES2_POINTERS];

	void* indicesPtr;
	AttribPointer** attribs;

	void* theBufferAddress;
	unsigned int locationInBuffer;
	int bufferSize;
	void* returnAddress;

	ParserBuffer& buffer;
	GLproxyContext* theContext;

	SharedMemory* androidSharedMemory;
};
#endif /* OPENGLES2PARSER_H_ */
