#ifndef GLPROXY_CONTEXT_H
#define GLPROXY_CONTEXT_H

#include "AttribPointer.h"
#include <vector>
using namespace std;

typedef struct
{
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

	theSurfaceStruct* getSurface(int id);

	AttribPointer* findAttribute(GLint index, bool createNew);

 private:
	unsigned int contextID;
	int windowID;
	vector<AttribPointer*> attribs;
	theSurfaceStruct surfaces[2];
};

#endif
