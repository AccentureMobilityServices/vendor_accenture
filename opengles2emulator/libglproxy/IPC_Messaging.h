/*
 * IPC_Messaging.h
 *
 *  Created on: 12 Oct 2011
 *      Author: jose.m.commins
 */

#ifndef IPC_MESSAGING_H_
#define IPC_MESSAGING_H_

#include <mqueue.h>
#include <time.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>


class IPC_Messaging
{
public:
	IPC_Messaging();
	virtual ~IPC_Messaging();
	mqd_t open(const char *theMessageQueueName, mode_t theFileMode);
	mqd_t receive_message(unsigned int thePriority);
	mqd_t receive_message_with_timeout(unsigned int thePriority, float theWaitTime);
	mqd_t send_message(char *theMessage, size_t theMessageLength, unsigned int thePriority);
	mqd_t send_message_with_timeout(char *theMessage, size_t theMessageLength, unsigned int thePriority, float theWaitTime);
	mqd_t get_attributes();
	mqd_t messages_in_queue();
	mqd_t set_blocking_mode(bool isBlocking);
	mqd_t set_notifier_thread(void *theThreadPointer);

	int bytes_received;
	char *theMessageBuffer;

private:
	char *theQueueName;
	int	 theMessageBufferSize;
	mqd_t theMessageQueue;
	struct mq_attr theQueueAttributes;
	struct sigevent theSignalThread;
};

#endif /* IPC_MESSAGING_H_ */
