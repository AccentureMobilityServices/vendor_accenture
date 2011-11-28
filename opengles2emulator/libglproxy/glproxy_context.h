#ifndef GLPROXY_CONTEXT_H
#define GLPROXY_CONTEXT_H

#include "AttribPointer.h"
#include <vector>
#include "debug.h"

using namespace std;

typedef struct theSurfaceStruct
{
	theSurfaceStruct() {
		surfaceEnumerator =0;
		surfacePhysicalAddress =0;
		width =0;
		height =0;

	}
	int	surfaceEnumerator;
	int pid;
	unsigned int surfacePhysicalAddress;
	unsigned int surfaceVirtualAddress;
	unsigned int width;
	unsigned int height;
	int pixelFormat;
	int pixelType;
	unsigned int stride;
} theSurfaceStruct;

class GLproxyContext {
 public:
	GLproxyContext(int contextID);

	void createContext();
	void destroyContext();
	bool isContext(int contextID);
	void switchToContext();

	theSurfaceStruct* getSurface();

	AttribPointer* findAttribute(GLint index, bool createNew);
	GLenum peekError();
	GLenum getError();

 private:
	unsigned int contextID;
	int windowID;
	vector<AttribPointer*> attribs;
	theSurfaceStruct lastSurface;
	int glError;
};

#endif
