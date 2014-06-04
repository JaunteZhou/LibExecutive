#include <string.h>
#include "CLMsgLoopManagerForShareNamedPipe.h"
#include "CLMessageReceiver.h"
#include "CLDataReceiverByNamedPipe.h"
#include "CLExecutiveNameServer.h"
#include "CLLogger.h"
#include "CLNotify.h"
#include "ErrorCode.h"
#include "CLBufferManager.h"
#include "CLMessagePoster.h"
#include "CLDataPostChannelByNamedPipeMaintainer.h"
#include "CLMessageDeserializer.h"
#include "CLMessageSerializer.h"
#include "CLProtocolDecapsulator.h"
#include "CLProtocolEncapsulator.h"
#include "CLInitialDataPostChannelNotifier.h"

using namespace std;

#define EXECUTIVE_CHANNEL_PATH "/tmp/"

CLMsgLoopManagerForShareNamedPipe::CLMsgLoopManagerForShareNamedPipe(CLMessageObserver *pMsgObserver, const char* pstrThreadName, CLProtocolEncapsulator *pEncapsulator, CLProtocolDecapsulator *pDecapsulator, CLMessageSerializer *pMsgSerializer, CLMessageDeserializer *pMsgDeserializer) : CLMessageLoopManager(pMsgObserver)
{
	if((pstrThreadName == 0) || (strlen(pstrThreadName) == 0) || (pMsgDeserializer == 0) || (pMsgSerializer == 0))
		throw "In CLMsgLoopManagerForShareNamedPipe::CLMsgLoopManagerForShareNamedPipe(), pstrThreadName or pMsgDeserializer or pMsgSerializer error";

	m_strThreadName = pstrThreadName;

	string strPath = EXECUTIVE_CHANNEL_PATH;
	strPath += pstrThreadName;

	//m_pEvent = new CLEvent(m_strThreadName.c_str(), true);
    m_pEvent = new CLNotify(EVENT, m_strThreadName.c_str());
	m_pMsgDeserializer = pMsgDeserializer;
	m_pMsgSerializer = pMsgSerializer;
	m_pDecapsulator = pDecapsulator;
	m_pEncapsulator = pEncapsulator;

	m_pMsgReceiver = new CLMessageReceiver(new CLBufferManager(), new CLDataReceiverByNamedPipe(strPath.c_str()), m_pMsgDeserializer, m_pDecapsulator);
}

CLMsgLoopManagerForShareNamedPipe::~CLMsgLoopManagerForShareNamedPipe()
{
}

CLStatus CLMsgLoopManagerForShareNamedPipe::Initialize()
{
	if(m_pMsgReceiver == 0)
		return CLStatus(-1, 0);

	CLMessagePoster *pMsgPoster = 0;

	try
	{
		CLExecutiveNameServer *pNameServer = CLExecutiveNameServer::GetInstance();
		if(pNameServer == 0)
		{
			CLLogger::WriteLogMsg("In CLMsgLoopManagerForShareNamedPipe::Initialize(), CLExecutiveNameServer::GetInstance error", 0);
			throw CLStatus(-1, 0);
		}

		string strPath = EXECUTIVE_CHANNEL_PATH;
		strPath += m_strThreadName;

		pMsgPoster = new CLMessagePoster(m_pMsgSerializer, m_pEncapsulator, new CLDataPostChannelByNamedPipeMaintainer(strPath.c_str()), m_pEvent);
		
		CLStatus s2 = pMsgPoster->Initialize(new CLInitialDataPostChannelNotifier(), 0);
		if(!s2.IsSuccess() && (s2.m_clErrorCode == DATA_POSTER_INITIALIZE_ERROR))
		{
			CLLogger::WriteLogMsg("In CLMsgLoopManagerForShareNamedPipe::Initialize(), pMsgPoster->Initialize error", 0);
			throw CLStatus(-1, 0);
		}

		CLStatus s = pNameServer->Register(m_strThreadName.c_str(), pMsgPoster);
		if(!s.IsSuccess())
		{
			CLLogger::WriteLogMsg("In CLMsgLoopManagerForShareNamedPipe::Initialize(), pNameServer->Register error", 0);

			delete m_pMsgReceiver;
			m_pMsgReceiver = 0;

			return CLStatus(-1, 0);
		}

		return CLStatus(0, 0);
	}
	catch(CLStatus& s1)
	{
		if(m_pMsgReceiver)
		{
			delete m_pMsgReceiver;
			m_pMsgReceiver = 0;
		}

		if(pMsgPoster)
			delete pMsgPoster;
		else
			delete m_pEvent;

		return s1;
	}
}

CLStatus CLMsgLoopManagerForShareNamedPipe::Uninitialize()
{
	delete m_pMsgReceiver;
	m_pMsgReceiver = 0;

	CLExecutiveNameServer *pNameServer = CLExecutiveNameServer::GetInstance();
	if(pNameServer == 0)
	{
		CLLogger::WriteLogMsg("In CLMsgLoopManagerForShareNamedPipe::Uninitialize(), CLExecutiveNameServer::GetInstance error", 0);
		return CLStatus(-1, 0);
	}

	return pNameServer->ReleaseCommunicationPtr(m_strThreadName.c_str());
}

CLStatus CLMsgLoopManagerForShareNamedPipe::WaitForMessage()
{
	CLStatus s = m_pEvent->Wait();
	if(!s.IsSuccess())
	{
		CLLogger::WriteLogMsg("In CLMsgLoopManagerForShareNamedPipe::WaitForMessage(), m_Event.Wait error", 0);
		return CLStatus(-1, 0);
	}

	long old_size = m_MessageContainer.size();

	CLStatus s1 = m_pMsgReceiver->GetMessage(m_MessageContainer);

	long new_size = m_MessageContainer.size();

	if(new_size > old_size)
	{
		if(!(m_pEvent->GetEvent()->ReleaseSemaphore(new_size - old_size - 1).IsSuccess()))
			CLLogger::WriteLogMsg("In CLMsgLoopManagerForShareNamedPipe::WaitForMessage(), m_pEvent->ReleaseSemaphore error; but may be made by the sequence of sendmsg.set.(wait).sendmsg(compute new_size).set", 0);
	}

	return s1;
}
