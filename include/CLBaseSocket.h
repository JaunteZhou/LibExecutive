#ifndef CLBaseSocket_H
#define CLBaseSocket_H

#include "CLStatus.h"

class CLIOVectors;
class CLSocketAddress;

class CLBaseSocket
{
public:
	explicit CLBaseSocket(bool bBlock);
	CLBaseSocket(int SocketFd, bool bBlock);
	virtual ~CLBaseSocket();

	virtual int GetSocket();

	virtual CLStatus Read(CLIOVectors& IOVectors, CLSocketAddress *pSocketAddress);
	virtual CLStatus Write(CLIOVectors& IOVectors, CLSocketAddress *pSocketAddress);

protected:
	CLStatus ReadOrWrite(bool bWrite, CLIOVectors& IOVectors, CLSocketAddress *pSocketAddress);
	CLStatus InitialSocket(int SocketFd, bool bBlock);

private:
	CLBaseSocket(const CLBaseSocket&);
	CLBaseSocket& operator=(const CLBaseSocket&);

protected:
	int m_SocketFd;
	bool m_bBlock;
};

#endif