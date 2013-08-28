#ifndef CLMessageReceiverBySTLqueue_H
#define CLMessageReceiverBySTLqueue_H

#include <queue>
#include "CLStatus.h"
#include "CLMutex.h"
#include "CLEvent.h"
#include "CLMessageReceiver.h"

class CLMessage;

/*
�������̰߳�ȫ��
��Ҫ��CLMsgLoopManagerForMsgQueue ���ʹ�ã�����������Ҫ�Ӷ��з��䣬�Ҳ��õ���delete
*/
class CLMessageReceiverBySTLqueue : public CLMessageReceiver
{
public:
	/*
	���캯����������������ʱ�����׳��ַ������͵��쳣
	*/
	CLMessageReceiverBySTLqueue();
	virtual ~CLMessageReceiverBySTLqueue();

public:
	CLStatus PushMessage(CLMessage * pMessage);
	virtual CLMessage* GetMessage();

private:
	CLMessageReceiverBySTLqueue(const CLMessageQueueBySTLqueue&);
	CLMessageReceiverBySTLqueue& operator=(const CLMessageQueueBySTLqueue&);

private:
	std::queue<CLMessage*> m_MessageQueue;
	CLMutex m_Mutex;
};

#endif