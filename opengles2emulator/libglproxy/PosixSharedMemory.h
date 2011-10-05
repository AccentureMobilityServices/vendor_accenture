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

#ifndef POSIXSHAREDMEMORY_H_
#define POSIXSHAREDMEMORY_H_


#include <sys/mman.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdio.h>


class PosixSharedMemory
{
public:
	PosixSharedMemory(const char *theSharedMemoryName);
	virtual ~PosixSharedMemory();
	void mapMemory(int sizeToMap);
	int GetSharedMemoryFileSize(int fileDescriptor);

	int fileDescriptor;
	void *mmappedAddress;

private:

};

#endif /* POSIXSHAREDMEMORY_H_ */
