/*
 * Copyright (C) 2011 Accenture Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Code to allow GLES2 demo app run natively on Linux

#include "glheaders.h"
#include <iostream>
#include <string.h>

void renderFrame(float rotx, float roty, float rotz);
void setupGraphics(int,int);

using namespace std;


void setShaders(const char* vertexShader, const char* fragShader);
void setModel(const char* objFile);
char* loadFromFile(const char* filename);
float rotx, roty, rotz;

void removeIncompatibleElement(char* shader, char* ident) {
	char* replace;
	while (replace = strstr(shader, ident)) {
		memset(replace, ' ', strlen(ident));
	}

}

void removeIncompatibleElements(char* shader) {
	removeIncompatibleElement(shader, "highp ");
	removeIncompatibleElement(shader, "mediump ");
	removeIncompatibleElement(shader, "lowp ");
}



void InitGL ()     
{
	char* vsh = loadFromFile("../assets/shader.vsh");
	char* fsh = loadFromFile("../assets/shader.fsh");
	removeIncompatibleElements(vsh);
	removeIncompatibleElements(fsh);
	char* model = loadFromFile("../assets/object.obj");
	setShaders(vsh, fsh);
	setModel(model);
	delete vsh;
	delete fsh;
	delete model;
	
	setupGraphics(500,500);
<<<<<<< HEAD
=======
	rotx = 0.0f;
	roty = 0.0f;
	rotz = 0.0f;
>>>>>>> hostNativeOpenGL
}        

void display ( void )   // Create The Display Function
{
<<<<<<< HEAD
	renderFrame();
=======
	renderFrame(rotx, roty, rotz);
	roty+=.5f;
>>>>>>> hostNativeOpenGL
  glutSwapBuffers ( );
  // Swap The Buffers To Not Be Left With A Clear Screen
}

void reshape ( int width , int height )   // Create The Reshape Function (the viewport)
{
	if (height==0)													// Prevent A Divide By Zero By
	{
		height=1;													// Making Height Equal One
	}

	glViewport(0,0,width,height);									// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);									// Select The Projection Matrix
	glLoadIdentity();												// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,1.0f,1000.0f);	// View Depth of 1000

	glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity();												// Reset The Modelview Matrix
}

void keyboard ( unsigned char key, int x, int y )  // Create Keyboard Function
{
  switch ( key ) {
    case 27:        // When Escape Is Pressed...
      exit ( 0 );   // Exit The Program
      break;        // Ready For Next Case
    default:        // Now Wrap It Up
      break;
  }
}

void arrow_keys ( int a_keys, int x, int y )  // Create Special Function (required for arrow keys)
{
  switch ( a_keys ) {
    case GLUT_KEY_UP:     // When Up Arrow Is Pressed...
      glutFullScreen ( ); // Go Into Full Screen Mode
      break;
    case GLUT_KEY_DOWN:               // When Down Arrow Is Pressed...
      glutReshapeWindow ( 500, 500 ); // Go Into A 500 By 500 Window
      break;
    default:
      break;
  }
}


int main ( int argc, char** argv )   // Create Main Function For Bringing It All Together
{
	glutInit            ( &argc, argv ); // Erm Just Write It =)
<<<<<<< HEAD
	glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE ); // Display Mode
=======
	glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // Display Mode
>>>>>>> hostNativeOpenGL
	glutInitWindowSize  ( 500, 500 ); // If glutFullScreen wasn't called this is the window size
	glutCreateWindow    ( "Native Window" ); // Window Title (argv[0] for current directory as title)
	InitGL ();
	glutDisplayFunc     ( display );  // Matching Earlier Functions To Their Counterparts
	glutReshapeFunc     ( reshape );
	glutKeyboardFunc    ( keyboard );
	glutSpecialFunc     ( arrow_keys );
	glutIdleFunc		  ( display );
	glutMainLoop        ( );          // Initialize The Main Loop
	return 0;
}

