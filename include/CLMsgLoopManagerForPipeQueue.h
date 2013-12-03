#ifndef CLMsgLoopManagerForPipeQueue_H
#define CLMsgLoopManagerForPipeQueue_H

#include <string>
#include "CLMessageLoopManager.h"
#include "CLMsgLoopManagerForPipeQueue.h"
#include "CLEvent.h"
#include "CLMessageReceiver.h"

using namespace std;

class CLMessageDeserializer;
class CLMultiMsgDeserializer;
class CLNamedPipe;

#define PIPE_QUEUE_BETWEEN_PROCESS 0
#define PIPE_QUEUE_IN_PROCESS 1

class CLMsgLoopManagerForPipeQueue : public CLMessageLoopManager
{
public:
	CLMsgLoopManagerForPipeQueue(CLMessageObserver *pMsgObserver, const char* pstrThreadName, int PipeQueueType, CLMessageDeserializer *pMsgDeserializer = NULL);
	virtual ~CLMsgLoopManagerForPipeQueue();

	CLStatus RegisterDeserializer(unsigned long lMsgID, CLMessageDeserializer *pDeserializer);

protected:
	virtual CLStatus Initialize();
	virtual CLStatus Uninitialize();

	virtual CLStatus WaitForMessage();

private:
	CLMsgLoopManagerForPipeQueue(const CLMsgLoopManagerForPipeQueue&);
	CLMsgLoopManagerForPipeQueue& operator=(const CLMsgLoopManagerForPipeQueue&);

private:
	CLMessageReceiver *m_pMsgReceiver;
	string m_strThreadName;
	CLEvent *m_pEvent;
	bool m_bIsShared;
	// CLNamedPipe *m_pNamedPipe;
	CLMessageDeserializer *m_pMultiMsgDeserializer;	
};

#endif