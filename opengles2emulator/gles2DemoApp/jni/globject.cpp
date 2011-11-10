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

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "globject.h"

using namespace std;

GlObject::GlObject() {
    
}




static inline Vector3D Vector3DMakeWithStartAndEndPoints(Vertex3D start, Vertex3D end)
{
    Vector3D ret;
    ret.x = end.x - start.x;
    ret.y = end.y - start.y;
    ret.z = end.z - start.z;
    //ret.Normalize();
    return ret;
}

static inline float DotProduct(Vertex3D center, Vertex3D v2, Vertex3D v3)
{
    Vector3D u = Vector3DMakeWithStartAndEndPoints(center, v3);
    Vector3D v = Vector3DMakeWithStartAndEndPoints(center, v2);
    return u.x*v.x + u.y*v.y + u.z*v.z;
}

static inline Vector3D Triangle3DCalculateSurfaceNormal(Triangle3D triangle)
{
    Vector3D u = Vector3DMakeWithStartAndEndPoints(triangle.v1, triangle.v2);
    Vector3D v = Vector3DMakeWithStartAndEndPoints(triangle.v1, triangle.v3);
    
    Vector3D ret;
    ret.x = (u.y * v.z) - (u.z * v.y);
    ret.y = (u.z * v.x) - (u.x * v.z);
    ret.z = (u.x * v.y) - (u.y * v.x);
    return ret;
}


void GlObject::calculateNormals()
{
    // Calculate surface normals and keep running sum of vertex normals
    surfaceNormals = (Vector3D*)calloc(numTriangles, sizeof(Vector3D));
    vertexNormals = (Vector3D*)calloc(numVertices, sizeof(Vector3D));
    
    for (int i = 0; i < numTriangles; i++)
    {
        
        Triangle3D t;
        int vecInd1 = faceIndices[i*3];
        int vecInd2 = faceIndices[i*3+1];
        int vecInd3 = faceIndices[i*3+2];
        
        t.v1.x = rv[vecInd1*3];
        t.v1.y = rv[vecInd1*3+1];
        t.v1.z = rv[vecInd1*3+2];
        
        t.v2.x = rv[vecInd2*3];
        t.v2.y = rv[vecInd2*3+1];
        t.v2.z = rv[vecInd2*3+2];
     
        t.v3.x = rv[vecInd3*3];
        t.v3.y = rv[vecInd3*3+1];
        t.v3.z = rv[vecInd3*3+2];
        
        //surfaceNormals[i].x = rvn[normalIndices[i*3+1]];//Triangle3DCalculateSurfaceNormal(t);
        //surfaceNormals[i].y = rvn[normalIndices[i*3+1]+1];
        //surfaceNormals[i].z = rvn[normalIndices[i*3+1]+2];
        
        surfaceNormals[i] = Triangle3DCalculateSurfaceNormal(t);
        
        //surfaceNormals[i].Normalize();
        //surfaceNormals[i].print();
        
        Vector3D a = Vector3DMakeWithStartAndEndPoints(t.v1, t.v2);
        Vector3D b = Vector3DMakeWithStartAndEndPoints(t.v2, t.v3);
        Vector3D c = Vector3DMakeWithStartAndEndPoints(t.v3, t.v1);
        
        float a2 = a.len2();
		float b2 = b.len2();
		float c2 = c.len2();
		
		//float area = .25f * sqrt(4.0f*a2*b2 - SQR(a2+b2-c2));	
        
         float angle = acos(DotProduct(t.v1,t.v2,t.v3));
        vertexNormals[vecInd1].add(surfaceNormals[i],1.0f); //area);
        angle = acos(DotProduct(t.v2,t.v3,t.v1));
        vertexNormals[vecInd2].add(surfaceNormals[i],1.0f);//area);
        angle = acos(DotProduct(t.v3,t.v1,t.v2));
        vertexNormals[vecInd3].add(surfaceNormals[i],1.0f);//area);
	


		
    }


// Loop through vertices again, dividing those that are used in multiple faces by the number of faces they are used in
    for (int i = 0; i < numVertices; i++)
    {
        vertexNormals[i].divideByNumberAdded();
        vertexNormals[i].Normalize();
        //vertexNormals[i].print();
    }
	BoundingBox bb;

	for (int i=0;i<numVertices;i++) {
		bb.AdjustBB((Vertex3D&)*(&rv[i*3]));
	}	

	float mx = bb.maxx - bb.minx; //fmax(abs(bb.minx),abs(bb.maxx));
	float my = bb.maxy - bb.miny; //fmax(abs(bb.miny),abs(bb.maxy));
	float mz = bb.maxz - bb.minz; //fmax(abs(bb.minz),abs(bb.maxz));

	float omax = fmax(mx,my);
	omax = fmax(omax,mz);	
	LOGI("max val = %f\n",omax);
    for (int i = 0; i < numVertices*3; i++){
		rv[i] /= omax;
	}
	
}

void GlObject::adjustBB()
{
	BoundingBox bb;

	for (int i=0;i<numTriangles*3;i++) {
		bb.AdjustBB((Vertex3D&)*(&ev[i*3]));
	}	

	float mx = bb.maxx - bb.minx; //fmax(abs(bb.minx),abs(bb.maxx));
	float my = bb.maxy - bb.miny; //fmax(abs(bb.miny),abs(bb.maxy));
	float mz = bb.maxz - bb.minz; //fmax(abs(bb.minz),abs(bb.maxz));

	float omax = fmax(mx,my);
	omax = fmax(omax,mz);	
	LOGI("max val = %f\n",omax);
    for (int i = 0; i < numTriangles*3; i++){
		ev[i*3] /= omax;
		ev[i*3+1] /= omax;
		ev[i*3+2] /= omax;
	}
}

void GlObject::loadObject(char* filedata) {
    int vertexcount = 0;
    int facecount = 0;
    int vertexnormalcount =0;
    int texcount =0;
    char* line;
	char* token;
	char* data = new char[strlen(filedata)+1];
	strcpy(data, filedata);

	char* lineSave = NULL;
	char* tokenSave = NULL;
	line = strtok_r(filedata, "\n", &lineSave);
	//printf("line %s\n", line);
	while (line!=NULL) {
			char buf[100];
			strcpy (buf, line);
			tokenSave = NULL;
			token = strtok_r(buf, "/ ", &tokenSave);
			
        	if (strcmp(token, "v")==0) {
            	vertexcount++;
        	} else if (strcmp(token,"f")==0) {
            	facecount++;
        	} else if (strcmp(token,"vn")==0) {
            	vertexnormalcount++;
        	} else if (strcmp(token,"vt")==0) {
            	texcount++;
       		 } else {
       	 	}
		line = strtok_r(NULL,"\n", &lineSave);
    }
    
    
    rv = new float[vertexcount*3];
    rvn = new float[vertexnormalcount*3]; // normals
    rt = new float[texcount*2]; // texture coordinates 
    
    // only support triangles (for now)
    faceIndices = new unsigned short[facecount*3];
    normalIndices = new unsigned short[facecount*3];
    textureIndices = new unsigned short[facecount*3];
    int vertexIndex = 0;
    int vertexNormalIndex = 0;
	int texIndex = 0;
    int faceIndex = 0;
 
	lineSave = NULL;
	line = strtok_r(data, "\n", &lineSave);
		
	while (line!=NULL) {
		token = strtok_r(line, "/ ", &tokenSave);
        if (strcmp(token, "v")==0) {
            rv[vertexIndex++] = atof(strtok_r(NULL," ", &tokenSave));
            rv[vertexIndex++] = atof(strtok_r(NULL," ", &tokenSave));
            rv[vertexIndex++] = atof(strtok_r(NULL," ", &tokenSave));
        } else if (strcmp(token,"f")==0) {
            for (int i=0;i<3;i++) {
                faceIndices[faceIndex] = atoi(strtok_r(NULL," /", &tokenSave))-1;
                textureIndices[faceIndex] = atoi(strtok_r(NULL," /", &tokenSave))-1;
                normalIndices[faceIndex] = atoi(strtok_r(NULL," /", &tokenSave))-1;
                faceIndex++;
            }
        } else if (strcmp(token,"vn")==0) {
            rvn[vertexNormalIndex++] = atof(strtok_r(NULL," ", &tokenSave));
            rvn[vertexNormalIndex++] = atof(strtok_r(NULL," ", &tokenSave));
            rvn[vertexNormalIndex++] = atof(strtok_r(NULL," ", &tokenSave));
        } else if (strcmp(token,"vt")==0) {
            rt[texIndex++] = atof(strtok_r(NULL," ", &tokenSave));
            rt[texIndex++] = atof(strtok_r(NULL," ", &tokenSave));
       	}
		line = strtok_r(NULL,"\n", &lineSave);

    }
    numTriangles = facecount;
    numVertices = vertexcount;

	ev = new GLfloat[facecount*3*3];
	en = new GLfloat[facecount*3*3];
	et = new GLfloat[facecount*3*2];
    
    for (int i=0;i<facecount*3;i++) {
        int ind = faceIndices[i];
        int normInd = normalIndices[i];
        int texInd = textureIndices[i];
		for (int j=0;j<3;j++) {
        	ev[i*3+j] = rv[ind*3+j];
        	en[i*3+j] = rvn[normInd*3+j];
		}
		for (int j=0;j<2;j++) {
        	et[i*2+j] = rt[texInd*2+j];
		}
    }

	adjustBB();
    //for (int i=0;i<10;i++) {
     //  printf("vertex: %f %f %f\n", ev[i*3], ev[i*3+1], ev[i*3+2]);
     //  printf("normal: %f %f %f\n", en[i*3], en[i*3+1], en[i*3+2]);
   // }
    
	delete[]data;
    
}


void GlObject::renderObject() {

    glVertexAttribPointer(glPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, ev);
    glEnableVertexAttribArray(glPositionHandle);
    glVertexAttribPointer(glNormalHandle, 3, GL_FLOAT, GL_FALSE, 0, en);
    glEnableVertexAttribArray(glNormalHandle);
    glVertexAttribPointer(glTextureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0, et);
    glEnableVertexAttribArray(glTextureCoordHandle);

	int numElements = numTriangles;
	
   	//printf("number of elements %d\n", numElements); 
    glDrawArrays(GL_TRIANGLES, 0, numElements*3);
}

void GlObject::drawNormals() {
    
    glVertexAttribPointer(glPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, normalLines);
    glEnableVertexAttribArray(glPositionHandle);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_LINES, 0, numVertices*2);
    
}
