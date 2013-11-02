#include <vector>
#include "CLMessageReceiver.h"
#include "CLStatus.h"
#include "CLLogger.h"
#include "CLMessageDeserializer.h"
#include "CLProtoParser.h"
#include "CLBuffer.h"
#include "CLDataReceiver.h"
#include "CLMessage.h"
#include "CLIOVector.h"
#include "CLCriticalSection.h"

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

	// while(true)
	// {
	// 	CLMessage *pMsg = PopMessage();
	// 	if(pMsg == 0)
	// 		break;

	// 	delete pMsg;
	// }
}

CLStatus CLMessageReceiver::GetMessage(queue<CLMessage*> &MessageQueue)
{
	//CLIOVector revDataBufVec;
	// CLStatus s1 = m_pDataReceiver->GetData(revDataBufVec);
	CLStatus s1 = m_pDataReceiver->GetData(m_pDataBuffer); //deal with the usedlen in getdata()
	// get data by iovec, push into databuffer!!!
	//!!此处需要判断独到的内容 可能是连接断了，可能是读不到内容了，出错了，就要关闭连接 就要回收m_pDataReceiver了
	if(!s1.IsSuccess())
	{
		CLLogger::WriteLogMsg("In CLMessageReceiver::GetMessage(), m_pDataReceiver->GetData() error", 0);
		return s1;
		// return PopMessage();
	}

	int readLen = (int)s1.m_clReturnCode;
	if(readLen != 0)
	{
		m_pDataBuffer->AddUsedBufferLen(readLen);//stl queue ,return`s readlen is 0 cause it use pdatabuffer->writedata(0)
	}

	// CLStatus s = m_pDataBuffer->PushBackIOVecs(revDataBufVec);
	// if(!s.IsSuccess())
	// {
	// 	CLLogger::WriteLogMsg("In CLMessageReceiver::GetMessage(), m_pDataBuffer->PushBackIOVecs() error", 0);
	// 	return s;
	// }

	if(m_pProtoParser != NULL)
	{
		vector<CLIOVector *> vSerializedMsgs;
		//vector<SLSerializedMsgScope> SerializedMsgScopeVec;//存储协议解析切割完成，反序列化之前的一个个的协议。
	
		//int decapsulateStartIndex = m_pDataBuffer->DataStartIndex();

		CLStatus s2 = m_pProtoParser->Decapsulate(m_pDataBuffer, vSerializedMsgs);//get and add data startindex in Decapsulate()
		if(!s2.IsSuccess())
		{
			CLLogger::WriteLogMsg("In CLMessageReceiver::GetMessage(), m_pProtoParser->Decapsulate(), error", 0);
			return s2;
		}

		//int decapsulateLen = (int)(s2.m_clReturnCode);
		//m_pDataBuffer->DataStartIndex(decapsulateStartIndex + decapsulateLen);

		for (int i = 0; i < vSerializedMsgs.size(); ++i)
		{
			CLMessage *pMessage = 0;
			CLStatus s3 = m_pMsgDeserializer->Deserialize(*(vSerializedMsgs[i]), &pMessage);//to sure that the deserializer wheather have use the CLBuffer directly or just copy
			if(!s3.IsSuccess() || pMessage == 0)
			{
				CLLogger::WriteLogMsg("In CLMessageReceiver::GetMessage(), m_pMsgDeserializer->Deserializer() error or msg = 0", 0);
				return s3;
			}
			MessageQueue.push(pMessage); 
		}
		if(!vSerializedMsgs.empty())
		{
			CLIOVector *tmp = vSerializedMsgs.back();
			vSerializedMsgs.pop_back();
			delete tmp;
		}
	}	
	// else
	// {
	// 	CLIOVector dataIOVec;
	// 	m_pDataBuffer->GetDataIOVecs(dataIOVec);

	// 	CLMessage *pMessage = 0;
	// 	CLStatus s4 = m_pMsgDeserializer->Deserialize(dataIOVec, &pMessage);//!！注意这里的dataIOVec中可能要对应多条消息，怎么搞呢？ //new SLSerializedMsgScop :datastart --usedlen
	// 	//deal with the dataindex change in Deserializer
	// 	// cause this is no protoparser ,so use 0 to point protoparse and scope, and in deserializer:get data start and add startindex in deserializer
	// 	if(!s4.IsSuccess() || pMessage == 0)
	// 	{
	// 		CLLogger::WriteLogMsg("In CLMessageReceiver::GetMessage(), m_pMsgDeserializer->Deserializer() error error or msg = 0", 0);
	// 		return s4;
	// 	}
	// 	m_pDataBuffer->AddDataStartIndex(s4.m_clReturnCode);
	// 	MessageQueue.push(pMessage);		
	// }
	
	return CLStatus(0, 0);
}

// CLMessage* CLMessageReceiver::PopMessage()
// {
// 	CLCriticalSection cs(&m_Mutex);

// 	if(m_MessageQueue.empty())
// 		return 0;

// 	CLMessage *p = m_MessageQueue.front();
// 	m_MessageQueue.pop();
// 	return p;	
// }