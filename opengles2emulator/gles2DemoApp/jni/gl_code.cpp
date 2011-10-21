/*
 * Copyright (C) 2009 The Android Open Source Project
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


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "glheaders.h"
#include "globject.h"


char* gVertexShader;
char* gFragmentShader;
char* gModel;

void setShaders(const char* vertexShader, const char* fragShader) {
	// need to copy the data as the jni call will deallocate it before it returns
	gVertexShader = new char[strlen(vertexShader)+1];
	strcpy(gVertexShader,vertexShader);
	gFragmentShader = new char[strlen(fragShader)+1];
	strcpy(gFragmentShader,fragShader);
} 

void setModel(const char* objFile) {
	// need to copy the data as the jni call will deallocate it before it returns
	gModel = new char[strlen(objFile)+1];
	strcpy(gModel,objFile);
} 



char* loadFromFile(const char* filename) {
  FILE * pFile;
  unsigned long lSize;
  char * buffer;
  size_t result;

  pFile = fopen ( filename , "r" );
  if (pFile==NULL) {LOGE ("File error"); exit (1);}

  // obtain file size:
  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile);
  rewind (pFile);

  // allocate memory to contain the whole file:
  buffer = (char*) malloc (sizeof(char)*lSize+1);
  if (buffer == NULL) {LOGE ("Memory error"); exit (2);}

  // copy the file into the buffer:
  result = fread (buffer,1,lSize,pFile);
  buffer[lSize] = 0;
  if (result != lSize) {LOGE ("Reading error"); exit (3);}

  /* the whole file is now loaded in the memory buffer. */
   LOGV("file loaded \n");
  // terminate
  fclose (pFile);
   return buffer;
}



GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint gProgram = NULL;

void createProgram(const char* pVertexSource, const char* pFragmentSource) {
	

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return;
    }

    gProgram = glCreateProgram();
    if (gProgram) {
        glAttachShader(gProgram, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(gProgram, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(gProgram);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(gProgram, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(gProgram, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(gProgram, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(gProgram);
            gProgram = NULL;
        }
    }
}

GLuint gvPositionHandle;
GLuint gProjMatrixHandle;
GLuint gLightDirectionHandle;
GLint toonDiffuseColor, toonPhongColor, toonEdge, toonPhong;
GlObject* gObj = NULL;
GLfloat aspectRatio;

bool setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGV("setupGraphics(%d, %d)", w, h);


 	createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }

	if (gObj==NULL) {
		gObj = new GlObject;
		gObj->loadObject(gModel);
		delete [] gModel; // clean up text model file
		gModel = NULL;
		gProjMatrixHandle = glGetUniformLocation(gProgram, "modelViewProjMatrix");
		gLightDirectionHandle = glGetUniformLocation(gProgram, "lightDirection");
		gvPositionHandle = glGetAttribLocation(gProgram, "position");
		LOGV("position handle %u\n", gvPositionHandle);
    	gObj->SetPositionHandle(gvPositionHandle);
    	checkGlError("glGetAttribLocation");
		gObj->SetNormalHandle(glGetAttribLocation(gProgram, "normal"));
    	checkGlError("glGetAttribLocation");
	}

    glViewport(0, 0, w, h);
	aspectRatio = (float)h/(float)w;
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
    checkGlError("glViewport");

    return true;
}


void shutdown() {
	delete[] gVertexShader;
	gVertexShader = NULL;
	delete[] gFragmentShader;
	gFragmentShader = NULL;
	delete[] gModel;
	gModel = NULL;
	delete gObj;
	gObj = NULL;	
}

const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f,
        0.5f, -0.5f };

GLfloat gProjMatrix[16];

const GLfloat lightDirection[]={0,0,-1};
GLfloat rot = 25.0f;

void renderFrame(float rotX, float rotY, float rotZ) {
    static float grey;
    grey = 0.3f;

	if (grey > 1.0f) {
        grey = 0.0f;
    }
    glClearColor(0.2, 0.35, 0.60, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

	IdentityMatrix(gProjMatrix);

	if (aspectRatio > 1.0) {
		ScaleMatrix(gProjMatrix, 1.2f, 1.2f/aspectRatio, 1.2f);
	} else {
		ScaleMatrix(gProjMatrix, aspectRatio, 1.2f, 1.2f);
	}
	
	rotY += rot;
	RotateMatrix(gProjMatrix, rotX, 1, 0, 0);
	RotateMatrix(gProjMatrix, rotY, 0, 1, 0);
	RotateMatrix(gProjMatrix, rotZ, 0, 0, 1);

	rot+=1.0f;

	glUniformMatrix4fv(gProjMatrixHandle, 1, 0, gProjMatrix);
	 glUniform3fv(gLightDirectionHandle, 1, lightDirection);
	// render the scene	
	gObj->renderObject();
}

