/*
**
** Copyright 2011, Accenture Ltd
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

#include "SharedMemory.h"
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdio.h>

struct paramStruct {
	paramStruct() {
		fileDescriptor = -1;
		mapAddress = 0;
	}

	int fileDescriptor;
	void* mapAddress;
};

SharedMemory::SharedMemory(const char *theSharedMemoryName)
{
	params = new paramStruct;
	params->fileDescriptor = shm_open (theSharedMemoryName, (O_RDWR), (ALLPERMS));
}

SharedMemory::~SharedMemory()
{

}

void SharedMemory::mapMemory(int sizeToMap)
{
	params->mapAddress = mmap (0, sizeToMap, PROT_READ | PROT_WRITE, MAP_SHARED, params->fileDescriptor, 0);

}

bool SharedMemory::initialised()
{
	return params->fileDescriptor > 0;
}

void* SharedMemory::getMappedAddress() 
{
	return params->mapAddress;
}

int SharedMemory::GetSharedMemoryFileSize()
{
int theFileSize;
FILE *theFile;


	theFile = fdopen(params->fileDescriptor, "rw");
	fseek(theFile, 0L, SEEK_END);
	theFileSize = ftell(theFile);
	fseek(theFile, 0, SEEK_SET);

	return theFileSize;
}
