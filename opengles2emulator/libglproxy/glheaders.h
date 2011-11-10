#ifndef GLHEADERS_H
#define GLHEADERS_H

#define GL_GLEXT_PROTOTYPES 1
#if defined __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/freeglut.h>
#endif


#endif
