#include "CLMessageReceiver.h"
#include "CLStatus.h"
#include "CLLogger.h"
#include "CLMessageDeserializer.h"
#include "CLProtoParser.h"
#include "CLBuffer.h"

// iovector 提供返回连续空间的长度值 CLBuffer根据分配了空间的buf，和指定的对应长度，返回该buf的值。
using namespace std;

CLMessageReceiver::CLMessageReceiver(CLDataReceiver *pDataReceiver, CLProtoParser *pProtoParser, CLMessageDeserializer *pMsgDeserializer)
{
	try
	{
		if(pDataReceiver == NULL)
		{
			throw "In CLMessageReceiver::CLMessageReceiver(), pDataReceiver = NULL";
		}
		if(pMsgDeserializer == NULL)
		{
			throw "In CLMessageReceiver::CLMessageReceiver(), pMsgDeserializerManager = NULL";
		}

		m_pProtoParser = pProtoParser;
		m_pDataReceiver = pDataReceiver;
		m_pMsgDeserializer = pMsgDeserializer;
		m_pDataBuffer = new CLBuffer();
	}
	catch(const char* s)
	{

		if(m_pProtoParser != NULL)
		{
			delete m_pProtoParser;
			m_pProtoParser = NULL;
		}
		if(m_pDataReceiver != NULL)
		{
			delete m_pDataReceiver;
			m_pDataReceiver = NULL;
		}
		if(m_pMsgDeserializer != NULL)
		{
			delete m_pMsgDeserializer;
			m_pMsgDeserializer = NULL;
		}

		throw s;
	}
}

CLMessageReceiver::~CLMessageReceiver()
{
	
	if(m_pProtoParser != NULL)
	{
		delete m_pProtoParser;
		m_pProtoParser = NULL;
	}
	if(m_pDataReceiver != NULL)
	{
		delete m_pDataReceiver;
		m_pDataReceiver = NULL;
	}
	if(m_pMsgDeserializer != NULL)
	{
		delete m_pMsgDeserializer;
		m_pMsgDeserializer = NULL;
	}

	while(true)
	{
		CLMessage *pMsg = GetMessage();
		if(pMsg == 0)
			break;

		delete pMsg;
	}
}

CLMessage* CLMessageReceiver::GetMessage()
{
	CLStatus s1 = m_pDataReceiver->GetData(m_pDataBuffer);

	if(!s1.IsSuccess())
	{
		CLLogger::WriteLogMsg("In CLMessageReceiver::GetMessage(), m_pDataReceiver->GetData() error", 0);
		return PopMessage();
	}

	/*int readLen = (int)s1.m_clReturnCode;
	if(readLen == 0)
	{
		CLLogger::WriteLogMsg("In CLMessageReceiver::GetMessage(), Getdata len = 0", 0);
		return PopMessage();
	}

	m_pDataBuffer->AddUsedBufferLen(readLen);*/ //this part complete in data receiver


	if(m_pProtoParser != NULL)
	{
		vector<SLSerializedMsgScope> SerializedMsgScopeVec;//存储协议解析切割完成，反序列化之前的一个个的协议。
	
		//int decapsulateStartIndex = m_pDataBuffer->DataStartIndex();

		CLStatus s2 = m_pProtoParser->Decapsulate(m_pDataBuffer, /*decapsulateStartIndex,*/ SerializedMsgScopeVec);//get and add data startindex in Decapsulate(0)
		if(!s2.IsSuccess())
		{
			CLLogger::WriteLogMsg("In CLMessageReceiver::GetMessage(0, m_pProtoParser->Decapsulate(), error", 0);
			return PopMessage();
		}

		//int decapsulateLen = (int)(s2.m_clReturnCode);
		//m_pDataBuffer->DataStartIndex(decapsulateStartIndex + decapsulateLen);

		CLStatus s3 = m_pMsgDeserializer->Deserializer(m_pDataBuffer, SerializedMsgScopeVec, m_MessageQueue);
		if(!s3.IsSuccess())
		{
			CLLogger::WriteLogMsg("In CLMessageReceiver::GetMessage(), m_pMsgDeserializer->Deserializer() error", 0);
		}
	}
	else
	{
		CLStatus s4 = m_pMsgDeserializer->Deserializer(m_pDataBuffer, (vector<SLSerializedMsgScope>)0, m_MessageQueue); //new SLSerializedMsgScop :datastart --usedlen
		//cause this is no protoparser ,so use 0 to point protoparse and scope, and in deserializer:get data start and add startindex in deserializer
		if(!s4.IsSuccess())
		{
			CLLogger::WriteLogMsg("In CLMessageReceiver::GetMessage(), m_pMsgDeserializer->Deserializer() error", 0);
		}
	}
	

	return PopMessage();
}

CLMessage* CLMessageReceiver::PopMessage()
{
	CLCriticalSection cs(&m_Mutex);

	if(m_MessageQueue.empty())
		return 0;

	CLMessage *p = m_MessageQueue.front();
	m_MessageQueue.pop();
	return p;	
}