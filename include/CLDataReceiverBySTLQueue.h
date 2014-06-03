#ifndef CLDATARECEIVERBYSTLQUEUE_H
#define CLDATARECEIVERBYSTLQUEUE_H

#include "CLDataReceiver.h"

class CLSTLQueue;
class CLIOVector;

class CLDataReceiverBySTLQueue : public CLDataReceiver
{
public:
	CLDataReceiverBySTLQueue(CLSTLQueue* pDataSTLQueue);
	virtual ~CLDataReceiverBySTLQueue();

	virtual CLStatus GetData(CLIOVector &IOVec);

private:
	CLDataReceiverBySTLQueue(const CLDataReceiverBySTLQueue&);
	CLDataReceiverBySTLQueue& operator=(const CLDataReceiverBySTLQueue&);

private:
	CLSTLQueue* m_pDataSTLQueue;
};

#endif