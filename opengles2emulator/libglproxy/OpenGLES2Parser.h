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
#include <GL/gl.h>
#include "ParserCommon.h"
#define NUMBER_OF_GLES2_POINTERS 256

int initOpenGLES2Parser(void);
int parseGLES2Command(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize, int payloadSize);

#endif /* OPENGLES2PARSER_H_ */