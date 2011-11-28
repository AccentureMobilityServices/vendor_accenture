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
#include <windows.h>

struct paramStruct {
	paramStruct() {
		hMem = NULL;
		mapAddress = NULL;
	}
	HANDLE hMem;
	void* mapAddress;
};

SharedMemory::SharedMemory(const char *theSharedMemoryName)
{
	params = new paramStruct;
	params->hMem = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,   // read/write access
			FALSE,                 // do not inherit the name
			theSharedMemoryName);               // name of mapping object
}

bool SharedMemory::initialised() 
{
	return params->hMem!=NULL;
}

SharedMemory::~SharedMemory()
{
	UnmapViewOfFile(params->mapAddress);
	CloseHandle(params->hMem);
}

void SharedMemory::mapMemory(int sizeToMap)
{
	params->mapAddress = MapViewOfFile(params->hMem, // handle to map object
               FILE_MAP_ALL_ACCESS,  // read/write permission
               0,
               0,
               sizeToMap);
}

void* SharedMemory::getMappedAddress() {
	return params->mapAddress;
}

int SharedMemory::GetSharedMemoryFileSize()
{
	return -1;
}
