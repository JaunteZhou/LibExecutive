#ifndef CLMSGLOOPMANAGERFORLIBEVENT_H
#define CLMSGLOOPMANAGERFORLIBEVENT_H

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event.h>
#include <list>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <map>
#include <set>
#include "CLMessageLoopManager.h"
#include "CLMutex.h"

class CLMessageReceiver;
class CLMessagePoster;
class CLDataPostChannelMainter;

class CLMsgLoopManagerForLibevent : public CLMessageLoopManager
{
public:
    CLMsgLoopManagerForLibevent(CLMessageObserver *pMsgObserver, const char *pstrThreadName, bool m_bMultipleThread);
    virtual ~CLMsgLoopManagerForLibevent();

    CLStatus RegisterReadEvent(int fd, CLMessageReceiver *pMsgReceiver);
    CLStatus UnRegisterReadEvent(int fd);

    CLStatus RegisterWriteEvent(int fd, CLMessagePoster *pMsgPoster);
    CLStatus RegisterConnectEvent(int fd, CLDataPostChannelMainter *pChannel);

protected:
    virtual CLStatus Initialize();
    virtual CLStatus Uninitialize();

    virtual CLStatus WaitForMessage();

private:
    void ClearDeletedSet();

    CLStatus Internal_RegisterReadEvent(int fd, CLMessageReceiver *pMsgReceiver);
    CLStatus Internal_UnRegisterReadEvent(int fd);

    CLStatus Internal_RegisterWriteEvent(int fd, CLMessagePoster *pMsgPoster);

    CLStatus Internal_RegisterConnectEvent(int fd, CLDataPostChannelMainter *pChannel);

private:
    CLMsgLoopManagerForLibevent(const CLMsgLoopManagerForLibevent&);
    CLMsgLoopManagerForLibevent& operator=(const CLMsgLoopManagerForLibevent&);

private:
    std::string m_strThreadName;

    std::map<struct event*, CLMessageReceiver*> m_ReadSetMap;
    std::map<struct event*, CLMessagePoster*> m_WriteSetMap;
    std::map<struct event*, CLDataPostChannelMainter*> m_ChannelMap;
    std::set<struct event*> m_DeletedSet;

    CLMutex m_MutexForReadMap;
    CLMutex m_MutexForWriteMap;
    CLMutex m_MutexForDeletedSet;  
    CLMutex m_MutexForChannelMap;

    bool m_bMultipleThread;

    struct event_base *m_base;
};

#endif
