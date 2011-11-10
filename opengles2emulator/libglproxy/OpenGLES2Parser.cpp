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
#include "gles2_emulator_constants.h"
#include "AttribPointer.h"
#include "debug.h"
#include <string.h>
#include "glproxy_context.h"
/*
 * These should be global.
 */
GLES2Parser::GLES2Parser(ParserBuffer& buffer)
	: buffer(buffer)
{
	initOpenGLES2Parser();
}

int GLES2Parser::initOpenGLES2Parser(void)
{
	int i;
	indicesPtr = NULL;

	for (i = 0; i < NUMBER_OF_GLES2_POINTERS; i++)
	{
		parserGLES2FunctionPointers[i] = &GLES2Parser::parse_nullFunction;
	}

	attribs = new AttribPointer*[MAX_ATTRIBS];
	//set all the pointers to NULL
	for (int i=0; i<MAX_ATTRIBS;i++) {
		attribs[i] = NULL;
	}

	parserGLES2FunctionPointers[GLCLEAR] = &GLES2Parser::parse_glClear;
	parserGLES2FunctionPointers[GLCLEARCOLORX] = &GLES2Parser::parse_glClearColorx;
	parserGLES2FunctionPointers[GLCLEARCOLORF] = &GLES2Parser::parse_glClearColorf;
	parserGLES2FunctionPointers[GLCREATEPROGRAM] = &GLES2Parser::parse_glCreateProgram;
	parserGLES2FunctionPointers[GLCREATESHADER] = &GLES2Parser::parse_glCreateShader;
	parserGLES2FunctionPointers[GLSHADERSOURCE] = &GLES2Parser::parse_glShaderSource;
	parserGLES2FunctionPointers[GLUSEPROGRAM] = &GLES2Parser::parse_glUseProgram;
	parserGLES2FunctionPointers[GLVERTEXATTRIBPOINTER] = &GLES2Parser::parse_glVertexAttribPointer;
	parserGLES2FunctionPointers[GLENABLEVERTEXATTRIBARRAY] = &GLES2Parser::parse_glEnableVertexAttribArray;
	parserGLES2FunctionPointers[GLATTACHSHADER] = &GLES2Parser::parse_glAttachShader;
	parserGLES2FunctionPointers[GLLINKPROGRAM] = &GLES2Parser::parse_glLinkProgram;
	parserGLES2FunctionPointers[GLGETPROGRAMINFOLOG] = &GLES2Parser::parse_glGetProgramInfoLog;
	parserGLES2FunctionPointers[GLDELETEPROGRAM] = &GLES2Parser::parse_glDeleteProgram;
	parserGLES2FunctionPointers[GLGETATTRIBLOCATION] = &GLES2Parser::parse_glGetAttribLocation;
	parserGLES2FunctionPointers[GLCOMPILESHADER] = &GLES2Parser::parse_glCompileShader;
	parserGLES2FunctionPointers[GLDRAWARRAYS] = &GLES2Parser::parse_glDrawArrays;
	parserGLES2FunctionPointers[GLDRAWELEMENTS] = &GLES2Parser::parse_glDrawElements;
	parserGLES2FunctionPointers[GLBLENDCOLOR] = &GLES2Parser::parse_glBlendColor;
	parserGLES2FunctionPointers[GLBLENDEQUATION] = &GLES2Parser::parse_glBlendEquation;
	parserGLES2FunctionPointers[GLBLENDEQUATIONSEPARATE] = &GLES2Parser::parse_glBlendEquationSeparate;
	parserGLES2FunctionPointers[GLBLENDFUNC] = &GLES2Parser::parse_glBlendFunc;
	parserGLES2FunctionPointers[GLBLENDFUNCSEPARATE] = &GLES2Parser::parse_glBlendFuncSeparate;
	parserGLES2FunctionPointers[GLBINDATTRIBLOCATION] = &GLES2Parser::parse_glBindAttribLocation;
	parserGLES2FunctionPointers[GLGENTEXTURES] = &GLES2Parser::parse_glGenTextures;
	parserGLES2FunctionPointers[GLACTIVETEXTURE] = &GLES2Parser::parse_glActiveTexture;
	parserGLES2FunctionPointers[GLBINDTEXTURE] = &GLES2Parser::parse_glBindTexture;
	parserGLES2FunctionPointers[GLFRAMEBUFFERTEXTURE2D] = &GLES2Parser::parse_glFramebufferTexture2D;
	parserGLES2FunctionPointers[GLISTEXTURE] = &GLES2Parser::parse_glIsTexture;
	parserGLES2FunctionPointers[GLDELETETEXTURES] = &GLES2Parser::parse_glDeleteTextures;
	parserGLES2FunctionPointers[GLBUFFERDATA] = &GLES2Parser::parse_glBufferData;
	parserGLES2FunctionPointers[GLBUFFERSUBDATA] = &GLES2Parser::parse_glBufferSubData;
	parserGLES2FunctionPointers[GLCHECKFRAMEBUFFERSTATUS] = &GLES2Parser::parse_glCheckFramebufferStatus;
	parserGLES2FunctionPointers[GLBINDBUFFER] = &GLES2Parser::parse_glBindBuffer;
	parserGLES2FunctionPointers[GLBINDFRAMEBUFFER] = &GLES2Parser::parse_glBindFramebuffer;
	parserGLES2FunctionPointers[GLBINDRENDERBUFFER] = &GLES2Parser::parse_glBindRenderbuffer;
	parserGLES2FunctionPointers[GLCOLORMASK] = &GLES2Parser::parse_glColorMask;
	parserGLES2FunctionPointers[GLCOMPRESSEDTEXIMAGE2D] = &GLES2Parser::parse_glCompressedTexImage2D;
	parserGLES2FunctionPointers[GLCOMPRESSEDTEXSUBIMAGE2D] = &GLES2Parser::parse_glCompressedTexSubImage2D;
	parserGLES2FunctionPointers[GLCOPYTEXIMAGE2D] = &GLES2Parser::parse_glCopyTexImage2D;
	parserGLES2FunctionPointers[GLCOPYTEXSUBIMAGE2D] = &GLES2Parser::parse_glCopyTexSubImage2D;
	parserGLES2FunctionPointers[GLDELETEBUFFERS] = &GLES2Parser::parse_glDeleteBuffers;
	parserGLES2FunctionPointers[GLDELETEFRAMEBUFFERS] = &GLES2Parser::parse_glDeleteFramebuffers;
	parserGLES2FunctionPointers[GLDELETERENDERBUFFERS] = &GLES2Parser::parse_glDeleteRenderbuffers;
	parserGLES2FunctionPointers[GLDEPTHFUNC] = &GLES2Parser::parse_glDepthFunc;
	parserGLES2FunctionPointers[GLDEPTHMASK] = &GLES2Parser::parse_glDepthMask;
	parserGLES2FunctionPointers[GLDEPTHRANGEF] = &GLES2Parser::parse_glDepthRangef;
	parserGLES2FunctionPointers[GLDETACHSHADER] = &GLES2Parser::parse_glDetachShader;
	//parserGLES2FunctionPointers[GLDISABLEVERTEXATTRIBARRAY] = &GLES2Parser::parse_glDisableVertexAttribArray;
	parserGLES2FunctionPointers[GLFRAMEBUFFERRENDERBUFFER] = &GLES2Parser::parse_glFramebufferRenderbuffer;
	parserGLES2FunctionPointers[GLGENBUFFERS] = &GLES2Parser::parse_glGenBuffers;
	parserGLES2FunctionPointers[GLGENERATEMIPMAP] = &GLES2Parser::parse_glGenerateMipmap;
	parserGLES2FunctionPointers[GLGENFRAMEBUFFERS] = &GLES2Parser::parse_glGenFramebuffers;
	parserGLES2FunctionPointers[GLGENRENDERBUFFERS] = &GLES2Parser::parse_glGenRenderbuffers;
	//parserGLES2FunctionPointers[GLGETACTIVEATTRIB] = &GLES2Parser::parse_glGetActiveAttrib;
	//parserGLES2FunctionPointers[GLGETACTIVEUNIFORM] = &GLES2Parser::parse_glGetActiveUniform;
	//parserGLES2FunctionPointers[GLGETATTACHEDSHADERS] = &GLES2Parser::parse_glGetAttachedShaders;
	parserGLES2FunctionPointers[GLGETBOOLEANV] = &GLES2Parser::parse_glGetBooleanv;
	//parserGLES2FunctionPointers[GLGETBUFFERPARAMETERIV] = &GLES2Parser::parse_glGetBufferParameteriv;
	//parserGLES2FunctionPointers[GLGETFLOATV] = &GLES2Parser::parse_glGetFloatv;
	/*parserGLES2FunctionPointers[GLGETFRAMEBUFFERATTACHMENTPARAMETERIV] = &GLES2Parser::parse_glGetFramebufferAttachmentParameteriv;
	parserGLES2FunctionPointers[GLGETINTEGERV] = &GLES2Parser::parse_glGetIntegerv;
	parserGLES2FunctionPointers[GLGETRENDERBUFFERPARAMETERIV] = &GLES2Parser::parse_glGetRenderbufferParameteriv;
	parserGLES2FunctionPointers[GLGETSHADERPRECISIONFORMAT] = &GLES2Parser::parse_glGetShaderPrecisionFormat;
	parserGLES2FunctionPointers[GLGETSHADERSOURCE] = &GLES2Parser::parse_glGetShaderSource;
	parserGLES2FunctionPointers[GLGETTEXPARAMETERFV] = &GLES2Parser::parse_glGetTexParameterfv;
	parserGLES2FunctionPointers[GLGETTEXPARAMETERIV] = &GLES2Parser::parse_glGetTexParameteriv;
	parserGLES2FunctionPointers[GLGETUNIFORMFV] = &GLES2Parser::parse_glGetUniformfv;
	parserGLES2FunctionPointers[GLGETUNIFORMIV] = &GLES2Parser::parse_glGetUniformiv;
	parserGLES2FunctionPointers[GLGETVERTEXATTRIBFV] = &GLES2Parser::parse_glGetVertexAttribfv;
	parserGLES2FunctionPointers[GLGETVERTEXATTRIBIV] = &GLES2Parser::parse_glGetVertexAttribiv;
	parserGLES2FunctionPointers[GLGETVERTEXATTRIBPOINTERV] = &GLES2Parser::parse_glGetVertexAttribPointerv;*/
	parserGLES2FunctionPointers[GLISBUFFER] = &GLES2Parser::parse_glIsBuffer;
	parserGLES2FunctionPointers[GLISENABLED] = &GLES2Parser::parse_glIsEnabled;
	parserGLES2FunctionPointers[GLISFRAMEBUFFER] = &GLES2Parser::parse_glIsFramebuffer;
	parserGLES2FunctionPointers[GLISPROGRAM] = &GLES2Parser::parse_glIsProgram;
	parserGLES2FunctionPointers[GLISRENDERBUFFER] = &GLES2Parser::parse_glIsRenderbuffer;
	parserGLES2FunctionPointers[GLISSHADER] = &GLES2Parser::parse_glIsShader;
	parserGLES2FunctionPointers[GLLINEWIDTH] = &GLES2Parser::parse_glLineWidth;
	parserGLES2FunctionPointers[GLPIXELSTOREI] = &GLES2Parser::parse_glPixelStorei;
	parserGLES2FunctionPointers[GLPOLYGONOFFSET] = &GLES2Parser::parse_glPolygonOffset;
	//parserGLES2FunctionPointers[GLREADPIXELS] = &GLES2Parser::parse_glReadPixels;
	parserGLES2FunctionPointers[GLRELEASESHADERCOMPILER] = &GLES2Parser::parse_glReleaseShaderCompiler;
	parserGLES2FunctionPointers[GLRENDERBUFFERSTORAGE] = &GLES2Parser::parse_glRenderbufferStorage;
	parserGLES2FunctionPointers[GLSCISSOR] = &GLES2Parser::parse_glScissor;
	parserGLES2FunctionPointers[GLSHADERBINARY] = &GLES2Parser::parse_glShaderBinary;
	parserGLES2FunctionPointers[GLSTENCILFUNCSEPARATE] = &GLES2Parser::parse_glStencilFuncSeparate;
	parserGLES2FunctionPointers[GLSTENCILMASK] = &GLES2Parser::parse_glStencilMask;
	parserGLES2FunctionPointers[GLSTENCILMASKSEPARATE] = &GLES2Parser::parse_glStencilMaskSeparate;
	parserGLES2FunctionPointers[GLSTENCILOP] = &GLES2Parser::parse_glStencilOp;
	//parserGLES2FunctionPointers[GLSTENCILOPERATE] = &GLES2Parser::parse_glStencilOpSeparate;
	parserGLES2FunctionPointers[GLTEXIMAGE2D] = &GLES2Parser::parse_glTexImage2D;
	parserGLES2FunctionPointers[GLTEXPARAMETERF] = &GLES2Parser::parse_glTexParameterf;
	//parserGLES2FunctionPointers[GLTEXPARAMETERFV] = &GLES2Parser::parse_glTexParameterfv;
	parserGLES2FunctionPointers[GLTEXPARAMETERI] = &GLES2Parser::parse_glTexParameteri;
	//parserGLES2FunctionPointers[GLTEXPARAMETERIV] = &GLES2Parser::parse_glTexParameteriv;
	//parserGLES2FunctionPointers[GLTEXSUBIMAGE2D] = &GLES2Parser::parse_glTexSubImage2D;
	parserGLES2FunctionPointers[GLUNIFORM1F] = &GLES2Parser::parse_glUniform1f;
	parserGLES2FunctionPointers[GLUNIFORM1FV] = &GLES2Parser::parse_glUniform1fv;
	parserGLES2FunctionPointers[GLUNIFORM1I] = &GLES2Parser::parse_glUniform1i;
	parserGLES2FunctionPointers[GLUNIFORM1IV] = &GLES2Parser::parse_glUniform1iv;
	parserGLES2FunctionPointers[GLUNIFORM2F] = &GLES2Parser::parse_glUniform2f;
	parserGLES2FunctionPointers[GLUNIFORM2FV] = &GLES2Parser::parse_glUniform2fv;
	parserGLES2FunctionPointers[GLUNIFORM2I] = &GLES2Parser::parse_glUniform2i;
	parserGLES2FunctionPointers[GLUNIFORM2IV] = &GLES2Parser::parse_glUniform2iv;
	parserGLES2FunctionPointers[GLUNIFORM3F] = &GLES2Parser::parse_glUniform3f;
	parserGLES2FunctionPointers[GLUNIFORM3I] = &GLES2Parser::parse_glUniform3i;
	parserGLES2FunctionPointers[GLUNIFORM3IV] = &GLES2Parser::parse_glUniform3iv;
	parserGLES2FunctionPointers[GLUNIFORM4F] = &GLES2Parser::parse_glUniform4f;
	parserGLES2FunctionPointers[GLUNIFORM4FV] = &GLES2Parser::parse_glUniform4fv;
	parserGLES2FunctionPointers[GLUNIFORM4I] = &GLES2Parser::parse_glUniform4i;
	parserGLES2FunctionPointers[GLUNIFORM4IV] = &GLES2Parser::parse_glUniform4iv;
	parserGLES2FunctionPointers[GLUNIFORMMATRIX2FV] = &GLES2Parser::parse_glUniformMatrix2fv;
	parserGLES2FunctionPointers[GLUNIFORMMATRIX3FV] = &GLES2Parser::parse_glUniformMatrix3fv;
	parserGLES2FunctionPointers[GLVALIDATEPROGRAM] = &GLES2Parser::parse_glValidateProgram;
	parserGLES2FunctionPointers[GLVERTEXATTRIB1F] = &GLES2Parser::parse_glVertexAttrib1f;
	parserGLES2FunctionPointers[GLVERTEXATTRIB1FV] = &GLES2Parser::parse_glVertexAttrib1fv;
	parserGLES2FunctionPointers[GLVERTEXATTRIB2F] = &GLES2Parser::parse_glVertexAttrib2f;
	parserGLES2FunctionPointers[GLVERTEXATTRIB2FV] = &GLES2Parser::parse_glVertexAttrib2fv;
	parserGLES2FunctionPointers[GLVERTEXATTRIB3F] = &GLES2Parser::parse_glVertexAttrib3f;
	parserGLES2FunctionPointers[GLVERTEXATTRIB4F] = &GLES2Parser::parse_glVertexAttrib4f;
	parserGLES2FunctionPointers[GLVERTEXATTRIB4FV] = &GLES2Parser::parse_glVertexAttrib4fv;


	parserGLES2FunctionPointers[GLGETUNIFORMLOCATION] = &GLES2Parser::parse_glGetUniformLocation;
	parserGLES2FunctionPointers[GLUNIFORM3FV] = &GLES2Parser::parse_glUniform3fv;
	parserGLES2FunctionPointers[GLUNIFORMMATRIX4FV] = &GLES2Parser::parse_glUniformMatrix4fv;
	parserGLES2FunctionPointers[GLDISABLE] = &GLES2Parser::parse_glDisable;
	parserGLES2FunctionPointers[GLENABLE] = &GLES2Parser::parse_glEnable;
	parserGLES2FunctionPointers[GLVIEWPORT] = &GLES2Parser::parse_glViewport;
	parserGLES2FunctionPointers[GLGETERROR] = &GLES2Parser::parse_glGetError;
	parserGLES2FunctionPointers[GLGETSHADERIV] = &GLES2Parser::parse_glGetShaderiv;
	parserGLES2FunctionPointers[GLGETSHADERINFOLOG] = &GLES2Parser::parse_glGetShaderInfoLog;
	parserGLES2FunctionPointers[GLGETPROGRAMIV] = &GLES2Parser::parse_glGetProgramiv;
	parserGLES2FunctionPointers[GLFRONTFACE] = &GLES2Parser::parse_glFrontFace;
	parserGLES2FunctionPointers[GLCULLFACE] = &GLES2Parser::parse_glCullFace;
	return 0;

}

void GLES2Parser::setContext(GLproxyContext* context)
{
	theContext = context;
}

int GLES2Parser::parseGLES2Command(int payloadSize, void* returnAddress)
{
	int theGLCommandToProcess;
	this->returnAddress = returnAddress;

	theGLCommandToProcess = buffer.fetch_GLint();

	buffer.markPos();

	DBG_PRINT("(%s)  Command: %d]\n", __FUNCTION__, theGLCommandToProcess);
	int ret = -1;
	if (theGLCommandToProcess < NUMBER_OF_GLES2_POINTERS)
	{
		multifp func = parserGLES2FunctionPointers[theGLCommandToProcess];
		ret = (this->*func)();
		buffer.returnToMark();
		buffer.advance(payloadSize);
	}
	return ret;
}



