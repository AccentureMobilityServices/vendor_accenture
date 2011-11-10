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

class GLES2Parser {
public:
	GLES2Parser(ParserBuffer& buffer);

	int parseGLES2Command(int payloadSize, void* returnAddress);
	void setContext(GLproxyContext* context);
private:
	int initOpenGLES2Parser();
	
	int parse_nullFunction();
	int parse_glClear();
	int parse_glClearColorf();
	int parse_glClearColorx();
	int parse_glCreateProgram();
	int parse_glCreateShader();
	int parse_glShaderSource();
	int parse_glUseProgram();
	int parse_glVertexAttribPointer();
	int parse_glEnableVertexAttribArray();
	int parse_glAttachShader();
	int parse_glLinkProgram();
	int parse_glGetProgramInfoLog();
	int parse_glDeleteProgram();
	int parse_glGetAttribLocation();
	int parse_glCompileShader();
	int parse_glDrawArrays();
	int parse_glGetUniformLocation();
	int parse_glUniformMatrix4fv();
	int parse_glUniform3fv();
	int parse_glDisable();
	int parse_glEnable();
	int parse_glDrawElements();

	int parse_glBlendColor();
	int parse_glBlendEquation();
	int parse_glBlendEquationSeparate();
	int parse_glBlendFunc();
	int parse_glBlendFuncSeparate();
	int parse_glBindAttribLocation();
	int parse_glGenTextures();
	int parse_glActiveTexture();
	int parse_glBindTexture();
	int parse_glFramebufferTexture2D();
	int parse_glIsTexture();
	int parse_glDeleteTextures();

	int parse_glBufferData();
	int parse_glBufferSubData();
	int parse_glCheckFramebufferStatus();
	int parse_glBindBuffer();
	int	parse_glBindFramebuffer();
	int parse_glBindRenderbuffer();
	int parse_glColorMask();
	int parse_glCompressedTexImage2D();
	int parse_glCompressedTexSubImage2D();
	int parse_glCopyTexImage2D();
	int parse_glCopyTexSubImage2D();
	int parse_glDeleteBuffers();
	int	parse_glDeleteFramebuffers();
	int parse_glDeleteRenderbuffers();
	int parse_glDepthFunc();
	int parse_glDepthMask();
	int parse_glDepthRangef();
	int parse_glDetachShader();
	int parse_glDisableVertexAttribArray();
	int parse_glFramebufferRenderbuffer();
	int parse_glGenBuffers();
	int parse_glGenerateMipmap();
	int parse_glGenFramebuffers();
	int parse_glGenRenderbuffers();
	int parse_glGetActiveAttrib();
	int parse_glGetActiveUniform();
	int parse_glGetAttachedShaders();
	int parse_glGetBooleanv();
	int parse_glGetBufferParameteriv();
	int parse_glGetFloatv();
	int parse_glGetFramebufferAttachmentParameteriv();
	int	parse_glGetIntegerv();
	int parse_glGetRenderbufferParameteriv();
	int parse_glGetShaderPrecisionFormat();
	int parse_glGetShaderSource();
	int parse_glGetTexParameterfv();
	int parse_glGetTexParameteriv();
	int parse_glGetUniformfv();
	int parse_glGetUniformiv();
	int parse_glGetVertexAttribfv();
	int parse_glGetVertexAttribiv();
	int parse_glGetVertexAttribPointerv();
	int parse_glIsBuffer();
	int parse_glIsEnabled();
	int parse_glIsFramebuffer();
	int parse_glIsProgram();
	int parse_glIsRenderbuffer();
	int parse_glIsShader();
	int parse_glLineWidth();
	int parse_glPixelStorei();
	int parse_glPolygonOffset();
	int parse_glReadPixels();
	int parse_glReleaseShaderCompiler();
	int parse_glRenderbufferStorage();
	int parse_glScissor();
	int parse_glShaderBinary();
	int parse_glStencilFuncSeparate();
	int parse_glStencilMask();
	int parse_glStencilMaskSeparate();
	int parse_glStencilOp();
	int parse_glStencilOpSeparate();
	int parse_glTexImage2D();
	int parse_glTexParameterf();
	int parse_glTexParameterfv();
	int parse_glTexParameteri();
	int parse_glTexParameteriv();
	int parse_glTexSubImage2D();
	int parse_glUniform1f();
	int parse_glUniform1fv();
	int parse_glUniform1i();
	int parse_glUniform1iv();
	int parse_glUniform2f();
	int parse_glUniform2fv();
	int parse_glUniform2i();
	int parse_glUniform2iv();
	int parse_glUniform3f();
	int parse_glUniform3i();
	int parse_glUniform3iv();
	int parse_glUniform4f();
	int parse_glUniform4fv();
	int parse_glUniform4i();
	int parse_glUniform4iv();
	int parse_glUniformMatrix2fv();
	int parse_glUniformMatrix3fv();
	int parse_glValidateProgram();
	int parse_glVertexAttrib1f();
	int parse_glVertexAttrib1fv();
	int parse_glVertexAttrib2f();
	int parse_glVertexAttrib2fv();
	int parse_glVertexAttrib3f();
	int parse_glVertexAttrib3fv();
	int parse_glVertexAttrib4f();
	int parse_glVertexAttrib4fv();

	int parse_glViewport();
	int parse_glGetError();
	int parse_glGetShaderiv();
	int parse_glGetShaderInfoLog();
	int parse_glGetProgramiv();
	int parse_glFrontFace();
	int parse_glCullFace();

private:
	AttribPointer* findAttribute(GLint index, bool createIfNecessary);
	void setReturnVal(int val);
	void showError();

	typedef int (GLES2Parser::*multifp)(); 
	multifp parserGLES2FunctionPointers[NUMBER_OF_GLES2_POINTERS];

	void* indicesPtr;
	AttribPointer** attribs;

	void* theBufferAddress;
	unsigned int locationInBuffer;
	int bufferSize;
	void* returnAddress;

	ParserBuffer& buffer;
	GLproxyContext* theContext;
};
#endif /* OPENGLES2PARSER_H_ */
