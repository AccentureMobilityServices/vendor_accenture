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


#include <unistd.h>
#include "GLRenderer.h"
#include <unistd.h>
#include "glheaders.h" 

GLRenderer* render;

extern "C" void startGLProxy(void *);

// if idle function is not defined, glutCheckLoop (mac osx only) does not return
void idlefunc() {
}

void startGLProxy (void*)  
{
	render = new GLRenderer();
	
	int argc = 1;
	char * argv[1];
	argv[0] = (char*)"libglproxy";	
	glutInit            ( &argc, argv );
	// Choose a double buffered RGBA context with a depth buffer enabled
	glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize  ( 320, 480 ); 
	glutIdleFunc(idlefunc);
	render->GLEventLoop();
}


