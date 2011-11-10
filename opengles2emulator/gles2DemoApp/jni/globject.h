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

#ifndef Chess_globject_h
#define Chess_globject_h

#include "glheaders.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#ifndef MAXFLOAT 
#include <values.h>
#endif

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}



#pragma pack(4)
// Uniform index.
enum {
    UNIFORM_PROJMATRIX,
    UNIFORM_LIGHTDIRECTION,
    NUM_UNIFORMS
};
//GLint uniforms[NUM_UNIFORMS];

// Attribute index.
enum {
    ATTRIB_VERTEX,
    ATTRIB_NORMAL,
    NUM_ATTRIBUTES
};



class objLoader;

#define SQR(x) ((x)*(x))


class Vector3D {
public:
    Vector3D() {
        x = 0;
        y = 0;
        z = 0;

        numberAdded = 0;
    }
    
    
    GLfloat x;
    GLfloat y;
    GLfloat z;
    
    void Normalize() {
        float len = length();
        x /=len; 
        y /=len;
        z /=len;
    }
    
    void add(Vector3D& other, float angle) {
        x += other.x;// * angle;
        y += other.y;// * angle;
        z += other.z;// * angle;
        numberAdded=1;//+=angle;
    }
    
    void divideByNumberAdded() {
        if (numberAdded > 0) {
            x /= (float)numberAdded;
            y /= (float)numberAdded;
            z /= (float)numberAdded;
        }
    }
    
    inline float len2() {
        float len = SQR(x) + SQR(y) + SQR(z);
		return len;
	}
    
    
    inline float length() {
        float len = sqrt(len2());
        return len;    
    }
    
    void print() {
        printf("<%f, %f, %f> len: %f\n", x, y,z, length());
    }
    
private:
    float numberAdded;
};

class Vertex3D {
public:
    GLfloat x;
    GLfloat y;
    GLfloat z;

};


class Triangle3D {
public:
    Vertex3D v1;
    Vertex3D v2;
    Vertex3D v3;
};


class BoundingBox {
public:
    BoundingBox() {
        reset();   
    }
    
    void AdjustBB(Vertex3D& v) {
        if (v.x <minx) {
            minx = v.x;
        }
        if (v.x >maxx) {
            maxx = v.x;
        }
        
        if (v.y <miny) {
            miny = v.y;
        }
        if (v.y >maxy) {
            maxy = v.y;
        }
        
        if (v.z <minz) {
            minz = v.z;
        }
        if (v.z >maxz) {
            maxz = v.z;
        } 
    }
    
    void print() {
        printf("Boundingbox %f %f %f - %f %f %f\n",minx,miny,minz,maxx,maxy,maxz);
    }
    
    void reset() {
        minx = MAXFLOAT;
        maxx = -MAXFLOAT;
        miny = MAXFLOAT;
        maxy = -MAXFLOAT;
        minz = MAXFLOAT;
        maxz = -MAXFLOAT;
    }
    
    float minx;
    float maxx;
    float miny;
    float maxy;
    float minz;
    float maxz;
};






class GlObject {
public:
    GlObject();
    
    void loadObject(char* objName);
    void renderObject();
    void drawNormals();
	void SetPositionHandle(GLuint positionHandle) {glPositionHandle = positionHandle;}
	void SetNormalHandle(GLuint normalHandle) {glNormalHandle = normalHandle;}
	void SetTextureCoordHandle(GLuint textureHandle) {glTextureCoordHandle = textureHandle;}
	void adjustBB();
    
    
private:
    
    void calculateNormals();
    objLoader* objData;
    unsigned short* faceIndices;
    unsigned short* normalIndices;
    unsigned short* textureIndices;
    Vector3D* surfaceNormals;
    Vector3D* vertexNormals;
    
    int numTriangles;
    //Vertex3D* vertices;
    //Vertex3D* normals;
    GLfloat* rv;
    GLfloat* rvn;
    GLfloat* rt;
    GLfloat* ev; // expanded vertices
    GLfloat* en;// expanded normals
    GLfloat* et;// expanded textures 
    GLfloat* normalLines;
	GLuint glPositionHandle;
	GLuint glNormalHandle;
	GLuint glTextureCoordHandle;
    int numVertices;
    BoundingBox bb;
};



inline void IdentityMatrix(GLfloat *m)
{
    m[0 * 4 + 0] = 1.0f;
    m[1 * 4 + 0] = 0.0f;
    m[2 * 4 + 0] = 0.0f;
    m[3 * 4 + 0] = 0.0f;
    m[0 * 4 + 1] = 0.0f;
    m[1 * 4 + 1] = 1.0f;
    m[2 * 4 + 1] = 0.0f;
    m[3 * 4 + 1] = 0.0f;
    m[0 * 4 + 2] = 0.0f;
    m[1 * 4 + 2] = 0.0f;
    m[2 * 4 + 2] = 1.0f;
    m[3 * 4 + 2] = 0.0f;
    m[0 * 4 + 3] = 0.0f;
    m[1 * 4 + 3] = 0.0f;
    m[2 * 4 + 3] = 0.0f;
    m[3 * 4 + 3] = 1.0f;
}

// Adjust a 4x4 matrix to apply a scale.
inline void ScaleMatrix(GLfloat *m, GLfloat scalex, GLfloat scaley, GLfloat scalez)
{
    m[0 * 4 + 0] *= scalex;
    m[0 * 4 + 1] *= scalex;
    m[0 * 4 + 2] *= scalex;
    m[0 * 4 + 3] *= scalex;
    m[1 * 4 + 0] *= scaley;
    m[1 * 4 + 1] *= scaley;
    m[1 * 4 + 2] *= scaley;
    m[1 * 4 + 3] *= scaley;
    m[2 * 4 + 0] *= scalez;
    m[2 * 4 + 1] *= scalez;
    m[2 * 4 + 2] *= scalez;
    m[2 * 4 + 3] *= scalez;
}

// Adjust a 4x4 matrix to apply a translation.
inline void TranslateMatrix(GLfloat *m, GLfloat translatex, GLfloat translatey, GLfloat translatez)
{
    m[3 * 4 + 0] += m[0 * 4 + 0] * translatex + m[1 * 4 + 0] * translatey + m[2 * 4 + 0] * translatez;
    m[3 * 4 + 1] += m[0 * 4 + 1] * translatex + m[1 * 4 + 1] * translatey + m[2 * 4 + 1] * translatez;
    m[3 * 4 + 2] += m[0 * 4 + 2] * translatex + m[1 * 4 + 2] * translatey + m[2 * 4 + 2] * translatez;
    m[3 * 4 + 3] += m[0 * 4 + 3] * translatex + m[1 * 4 + 3] * translatey + m[2 * 4 + 3] * translatez;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Adjust a 4x4 matrix to apply a rotation.
inline void RotateMatrix(GLfloat *m, GLfloat angle, GLfloat vx, GLfloat vy, GLfloat vz)
{
    GLfloat len = sqrt(vx * vx + vy * vy + vz * vz);
    if (len != 0) {
        vx /= len;
        vy /= len;
        vz /= len;
    }

    GLfloat c, s, ic;
    c = cos(angle * M_PI / 180.0);
    s = sin(angle * M_PI / 180.0);
    ic = 1.0f - c;

    GLfloat rot[16];
    rot[0 * 4 + 0] = vx * vx * ic + c;
    rot[1 * 4 + 0] = vx * vy * ic - vz * s;
    rot[2 * 4 + 0] = vx * vz * ic + vy * s;
    rot[3 * 4 + 0] = 0.0f;
    rot[0 * 4 + 1] = vy * vx * ic + vz * s;
    rot[1 * 4 + 1] = vy * vy * ic + c;
    rot[2 * 4 + 1] = vy * vz * ic - vx * s;
    rot[3 * 4 + 1] = 0.0f;
    rot[0 * 4 + 2] = vx * vz * ic - vy * s;
    rot[1 * 4 + 2] = vy * vz * ic + vx * s;
    rot[2 * 4 + 2] = vz * vz * ic + c;
    rot[3 * 4 + 2] = 0.0f;
    rot[0 * 4 + 3] = 0.0f;
    rot[1 * 4 + 3] = 0.0f;
    rot[2 * 4 + 3] = 0.0f;
    rot[3 * 4 + 3] = 1.0f;

    GLfloat temp[16];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            temp[j * 4 + i] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                temp[j * 4 + i] += m[k * 4 + i] * rot[j * 4 + k];
            }
        }
    }

    memcpy(m, temp, sizeof(temp));
}
#endif
