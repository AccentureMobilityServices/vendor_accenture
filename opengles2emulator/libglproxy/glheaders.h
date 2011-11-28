#ifndef GLHEADERS_H
#define GLHEADERS_H

#ifdef WIN32
typedef int socklen_t;
#include "GLee.h"
#endif

#define GL_GLEXT_PROTOTYPES 1
#if defined __APPLE__
#include "GLee.h"
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include "GLee.h"
#include <GL/gl.h>
#include <GL/freeglut.h>
#endif


#endif
