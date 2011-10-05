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

void GlObject::loadObject(char* filedata) {
    int vertexcount = 0;
    int facecount = 0;
    int vertexnormalcount =0;
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
       		 } else {
       	 	}
		line = strtok_r(NULL,"\n", &lineSave);
    }
    
    
    rv = new float[vertexcount*3];
    rvn = new float[vertexnormalcount*3]; // normals from original obj file order
    rn = new float[vertexcount*3]; // normals rearranged in same order as vertices
    
    
    // only support triangles (for now)
    faceIndices = new unsigned short[facecount*3];
    normalIndices = new unsigned short[facecount*3];
    int vertexIndex = 0;
    int vertexNormalIndex = 0;
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
                
                atoi(strtok_r(NULL," /", &tokenSave));
                normalIndices[faceIndex] = atoi(strtok_r(NULL," /", &tokenSave))-1;
                faceIndex++;
            }
        } else if (strcmp(token,"vn")==0) {
            rvn[vertexNormalIndex++] = atof(strtok_r(NULL," ", &tokenSave));
            rvn[vertexNormalIndex++] = atof(strtok_r(NULL," ", &tokenSave));
            rvn[vertexNormalIndex++] = atof(strtok_r(NULL," ", &tokenSave));
       	}
		line = strtok_r(NULL,"\n", &lineSave);

    }
    numTriangles = facecount;
    numVertices = vertexcount;
    
    int* numNormsAdded = new int[vertexcount];
    for (int i=0;i<vertexcount;i++) {
        rn[i*3] = 0.0f;
        rn[i*3+1] = 0.0f;
        rn[i*3+2] = 0.0f;
        numNormsAdded[i] = 0;
    }
    
    for (int i=0;i<facecount*3;i++) {
        int ind = faceIndices[i];
        int normInd = normalIndices[i];
        numNormsAdded[ind]++;
        rn[ind*3] += rvn[normInd*3];
        rn[ind*3+1] += rvn[normInd*3+1];
        rn[ind*3+2] += rvn[normInd*3+2];
    }
    
    for (int i=0;i<vertexcount;i++) {
        
       int count = numNormsAdded[i];

       if (count > 1) {
            rn[i*3] /= (float)count;
            rn[i*3+1] /= (float)count;
            rn[i*3+2] /= (float)count;
            
        }
        //normalize
		Vector3D* v = (Vector3D*)&rn[i*3];
		v->Normalize();

    }
    
    calculateNormals();
    for (int i=0;i<numVertices;i++) {
		Vector3D& v = *(Vector3D*)&rn[i*3];
        v.x = vertexNormals[i].x;
        v.y = vertexNormals[i].y;
        v.z = vertexNormals[i].z;
        
    }
	
    
    normalLines = new float[numVertices*3*2];
    for (int i=0;i<numVertices;i++) {
        normalLines[i*6] = rv[i*3];
        normalLines[i*6+1] = rv[i*3+1];
        normalLines[i*6+2] = rv[i*3+2];
        normalLines[i*6+3] = rv[i*3] + rn[i*3]/5.0f;
        normalLines[i*6+4] = rv[i*3+1] + rn[i*3+1]/5.0f;
        normalLines[i*6+5] = rv[i*3+2] + rn[i*3+2]/5.0f;
    }

	delete[]data;
    
}


void GlObject::renderObject() {

    glVertexAttribPointer(glPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, rv);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(glPositionHandle);
    checkGlError("glEnableVertexAttribArray");
    glVertexAttribPointer(glNormalHandle, 3, GL_FLOAT, GL_FALSE, 0, rn);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(glNormalHandle);
    checkGlError("glEnableVertexAttribArray");
	
    
    glDrawElements(GL_TRIANGLES, numTriangles*3, GL_UNSIGNED_SHORT, faceIndices);
    checkGlError("glDrawArrays");


}

void GlObject::drawNormals() {
    
    glVertexAttribPointer(glPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, normalLines);
    glEnableVertexAttribArray(glPositionHandle);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_LINES, 0, numVertices*2);
    
}
