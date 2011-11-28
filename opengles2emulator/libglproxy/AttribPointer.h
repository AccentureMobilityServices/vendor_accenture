/*
** Copyright 2006, The Android Open Source Project
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

#ifndef ATTRIBPOINTER_H
#define ATTRIBPOINTER_H

class AttribPointer {
public:
	AttribPointer() {
		index = 0;
		size = 0;
		type = 0;
		normalized = 0;
		stride = 0;
		length = 0;
		pointer = 0;
		enabled = false;
	}
	GLint		index;
	GLint  		size;
 	GLenum  	type;
 	GLboolean  	normalized;
 	GLsizei  	stride;
	GLint		length;
 	GLvoid *pointer;
	GLboolean		enabled;
};
// Actually defined by implementation as GL_MAX_VERTEX_ATTRIBS - 32 should be more than enough
#define MAX_ATTRIBS 32


#endif // ATTRIBPOINTER_H

