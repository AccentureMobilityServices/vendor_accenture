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

#include "OpenGLES1_1Parser.h"
extern char tempDataStorage[256];

/*
 * These should be global.
 */
enum { gl_null = 0, cmd_glClear, cmd_glClearColorf, cmd_glClearColorx };


int (*parserGLES1_1FunctionPointers[NUMBER_OF_GLES1_1_POINTERS])(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize);


int initOpenGLES1_1Parser(void)
{
int i;


	for (i = 0; i < NUMBER_OF_GLES1_1_POINTERS; i++)
	{
		parserGLES1_1FunctionPointers[i] = parse_nullFunction;
	}

	parserGLES1_1FunctionPointers[cmd_glClear] = parse_glClear;
	parserGLES1_1FunctionPointers[cmd_glClearColorf] = parse_glClearColorf;
	parserGLES1_1FunctionPointers[cmd_glClearColorx] = parse_glClearColorx;

	return 0;

}

int parseGLES1_1Command(void *theBufferAddress, unsigned int *locationInBuffer, int bufferSize)
{
int theGLCommandToProcess;


	fetchBufferWrappedBytes(theBufferAddress, locationInBuffer, bufferSize, tempDataStorage, sizeof(int));
	theGLCommandToProcess = *(int *)tempDataStorage;

	printf("(%s)  Command: %d]\n", __FUNCTION__, theGLCommandToProcess);
	if (theGLCommandToProcess >= NUMBER_OF_GLES1_1_POINTERS)
	{
		return -1;
	}

	return parserGLES1_1FunctionPointers[theGLCommandToProcess](theBufferAddress, locationInBuffer, bufferSize);

}



