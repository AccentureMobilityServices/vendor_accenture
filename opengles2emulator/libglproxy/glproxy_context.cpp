#include <list>
#include <stdio.h>
#include "glheaders.h"
#include "glproxy_context.h"

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
}

void GLproxyContext::createContext()
{
	char buf[256];
	sprintf(buf, "glcontext %08x\n", contextID);
	windowID = glutCreateWindow(buf);
	glutDisplayFunc(displayfn);
	glutReshapeFunc(reshapefn);
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();

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
	return this->contextID == contextID;
}

void GLproxyContext::switchToContext() 
{
	glutSetWindow(windowID);
}

theSurfaceStruct* GLproxyContext::getSurface(int id)
{
	if (id<0 || id>1) {
		return NULL;
	}
	return &surfaces[id];
}

void GLproxyContext::destroyContext() 
{
	glutDestroyWindow(windowID);
}
