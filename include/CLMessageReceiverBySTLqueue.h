#ifndef CLMessageReceiverBySTLqueue_H
#define CLMessageReceiverBySTLqueue_H

#include "CLStatus.h"
#include "CLMessageReceiver.h"


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

	virtual CLMessage* GetMessageFromChannel();

private:
	CLMessageReceiverBySTLqueue(const CLMessageQueueBySTLqueue&);
	CLMessageReceiverBySTLqueue& operator=(const CLMessageQueueBySTLqueue&);

};

#endif