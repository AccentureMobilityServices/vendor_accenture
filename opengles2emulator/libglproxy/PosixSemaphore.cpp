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

#include "PosixSemaphore.h"
#include "debug.h"
#include "stdio.h"

PosixSemaphore::PosixSemaphore(const char *theSemaphoreName)
{
	thePOSIXSemaphore = sem_open (theSemaphoreName, O_RDWR, 0666, 1);
	if (thePOSIXSemaphore == SEM_FAILED)
	{
		DBG_PRINT("AAAAARGH!  Could not open semaphore!\n");
	}
}

PosixSemaphore::~PosixSemaphore()
{

}

int PosixSemaphore::getValue() {
    int value;
    sem_getvalue(thePOSIXSemaphore, &value);
    return value;
}

void PosixSemaphore::wait() {
    sem_wait(thePOSIXSemaphore);
}

int PosixSemaphore::tryWait() {
    return (sem_trywait(thePOSIXSemaphore));
}

void PosixSemaphore::post() {
    sem_post(thePOSIXSemaphore);
}
