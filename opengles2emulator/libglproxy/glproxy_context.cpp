#include <list>
#include <stdio.h>
#include "glheaders.h"
#include "glproxy_context.h"

bool gShowWindow=false;

// we don't want to do anything in reshape or display functions, but need to define
// them to keep glut happy and prevent it doing things like setting the viewport in
// reshape
void displayfn(void) {
}
void reshapefn(int width, int height) {
}

GLproxyContext::GLproxyContext(int contextID) 
{
	this->contextID = contextID;
	glError = GL_NO_ERROR;
}

void GLproxyContext::createContext()
{
	char buf[256];
	sprintf(buf, "glcontext %08x\n", contextID);
	windowID = glutCreateWindow(buf);
	glutHideWindow();
	glutDisplayFunc(displayfn);
	glutReshapeFunc(reshapefn);
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
	glutHideWindow();
}

AttribPointer* GLproxyContext::findAttribute(GLint index, bool createNew) 
{
	AttribPointer* ptr = NULL;
	vector<AttribPointer*>::iterator iter;
	bool found = false;
	for (iter = attribs.begin(); iter != attribs.end() ; iter++) {
		ptr = *iter;
		if (ptr->index == index) {
			found = true;
			break;
		}	
	}
   
	if (!found && createNew) {
		ptr = new AttribPointer;
		ptr->index = index;
		attribs.push_back(ptr);
	}
	return ptr;
}

bool GLproxyContext::isContext(int contextID)
{
	return this->contextID == (unsigned int)contextID;
}

void GLproxyContext::switchToContext() 
{
	glutSetWindow(windowID);
}

theSurfaceStruct* GLproxyContext::getSurface()
{
	return &lastSurface;
}

void GLproxyContext::destroyContext() 
{
	DBGPRINT("destroy window called\n");
	glutDestroyWindow(windowID);
}

GLenum GLproxyContext::peekError() 
{
	if (glError!= GL_NO_ERROR) 
	{
		return glError;
	}

	glError = glGetError();
	return glError;
}
GLenum GLproxyContext::getError() 
{
	GLenum errVal;
	if (glError!= GL_NO_ERROR) 
	{
		errVal = glError;
		glError = 0;
		return errVal;
	}

	return glGetError();
}
