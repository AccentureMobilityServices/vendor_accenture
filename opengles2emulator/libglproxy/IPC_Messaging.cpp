/*
 * IPC_Messaging.cpp
 *
 *  Created on: 12 Oct 2011
 *      Author: jose.m.commins
 */
#include "debug.h"
#include "IPC_Messaging.h"



IPC_Messaging::IPC_Messaging()
{

}

IPC_Messaging::~IPC_Messaging()
{
	if (theMessageQueue)
	{
		mq_close(theMessageQueue);
		mq_unlink(theQueueName);
		delete [] theMessageBuffer;
		delete [] theQueueName;
	}
}

mqd_t
IPC_Messaging::open(const char *theMessageQueueName, mode_t theFileMode)
{
	theMessageQueue = mq_open (theMessageQueueName, theFileMode);
	if (theMessageQueue > 0)
	{
		theQueueName = new char [strlen(theMessageQueueName)];
		get_attributes();
		theMessageBufferSize = theQueueAttributes.mq_msgsize;
		DBG_PRINT("Message queue size: %d", theMessageBufferSize);
		theMessageBuffer = new char [theMessageBufferSize];
	} else {
		DBG_PRINT("Could not open message queue.\n");
	}
	return theMessageQueue;
}


mqd_t
IPC_Messaging::receive_message(unsigned int thePriority)
{
	return mq_receive (theMessageQueue, theMessageBuffer, theMessageBufferSize, &thePriority);
}

mqd_t
IPC_Messaging::receive_message_with_timeout(unsigned int thePriority, float theWaitTime)
{
struct timespec theWait;

	theWait.tv_sec = (int)floor(theWaitTime);
	theWait.tv_nsec = (int)(1e9 * (theWaitTime - theWait.tv_sec));

	return mq_timedreceive (theMessageQueue, theMessageBuffer, theMessageBufferSize, &thePriority, &theWait);
}

mqd_t
IPC_Messaging::send_message(char *theMessage, size_t theMessageLength, unsigned int thePriority)
{
	return mq_send (theMessageQueue, theMessage, theMessageLength, thePriority);
}

mqd_t
IPC_Messaging::send_message_with_timeout(char *theMessage, size_t theMessageLength, unsigned int thePriority, float theWaitTime)
{
struct timespec theWait;

	theWait.tv_sec = (int)floor(theWaitTime);
	theWait.tv_nsec = (int)(1e9 * (theWaitTime - theWait.tv_sec));

	return mq_timedsend (theMessageQueue, theMessage, theMessageLength, thePriority, &theWait);
}

mqd_t
IPC_Messaging::get_attributes()
{
	return mq_getattr(theMessageQueue, &theQueueAttributes);
}

mqd_t
IPC_Messaging::messages_in_queue()
{
	if (!mq_getattr(theMessageQueue, &theQueueAttributes))
	{
		DBG_PRINT ("Error in obtaining queue attributes!\n");
		return -1;
	} else {
		return theQueueAttributes.mq_curmsgs;
	}
}

mqd_t
IPC_Messaging::set_blocking_mode(bool isBlocking)
{
	if (!mq_getattr(theMessageQueue, &theQueueAttributes))
	{
		DBG_PRINT ("Error in obtaining queue attributes!\n");
		return -1;
	} else {
		isBlocking ? theQueueAttributes.mq_flags = 0 : theQueueAttributes.mq_flags = O_NONBLOCK;
		return mq_setattr(theMessageQueue, &theQueueAttributes, NULL);
	}
}

mqd_t
IPC_Messaging::set_notifier_thread(void *theThreadPointer)
{
	theSignalThread.sigev_notify = SIGEV_THREAD;
	theSignalThread.sigev_notify_function = (void (*)(sigval_t))theThreadPointer;
	theSignalThread.sigev_notify_attributes = NULL;
	theSignalThread.sigev_value.sival_ptr = &theMessageQueue;
	return mq_notify(theMessageQueue, &theSignalThread);
}
