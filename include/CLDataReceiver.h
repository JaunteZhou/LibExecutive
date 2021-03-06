#ifndef CLDataReceiver_H
#define CLDataReceiver_H

#include "CLStatus.h"

class CLIOVectors;

class CLDataReceiver
{
public:
	CLDataReceiver();
	virtual ~CLDataReceiver();

	virtual CLStatus GetData(CLIOVectors& IOVectors, void *pContext) = 0;

private:
	CLDataReceiver(const CLDataReceiver&);
	CLDataReceiver& operator=(const CLDataReceiver&);
};

#endif