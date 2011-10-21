/*
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

#ifndef GLES2_H_
#define GLES2_H_
#include <stdio.h>
#include "glheaders.h"

int parse_glCreateProgram(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glCreateShader(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glShaderSource(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glUseProgram(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glVertexAttribPointer(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glEnableVertexAttribArray(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glAttachShader(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glLinkProgram(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glGetProgramInfoLog(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glDeleteProgram(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glGetAttribLocation(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glCompileShader(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glDrawArrays(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glGetUniformLocation(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glUniformMatrix4fv(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glUniform3fv(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glDisable(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glEnable(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glDrawElements(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);
int parse_glViewport(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);

#endif /* GLES2_H_ */
