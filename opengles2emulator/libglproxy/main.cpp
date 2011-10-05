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


#include "GLRenderer.h"
#include <unistd.h>
#include <GL/glut.h>

GLRenderer* render;

extern "C" void startGLProxy(void *);

void eventloop ( void ) 
{
	render->GLEventLoop();
}

void display ( void )   // Create The Display Function
{
	glutSwapBuffers();
}

void startGLProxy (void*)   // Create Main Function For Bringing It All Together
{
	render = new GLRenderer();
	
	int argc = 1;
	char *argv[1];
	argv[0] = "libglproxy";	
	glutInit            ( &argc, argv );
	glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // Display Mode
	glutInitWindowSize  ( 320, 480 ); // If glutFullScreen wasn't called this is the window size
	
	glutCreateWindow    ( "GLProxy" ); // Window Title (argv[0] for current directory as title)
	render->initializeGL();
	render->GLEventLoop();

}



