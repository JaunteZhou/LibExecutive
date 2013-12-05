#include <sys/timerfd.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#include "CLTimerFd.h"
#include "CLLogger.h"

CLTimerFd::CLTimerFd(struct itimerspec& sTimerValue, string pstrRemoteName, const int& ID)
{
	m_strRemoteName = pstrRemoteName;
	m_ID = ID;
	if(sTimerValue.it_interval.tv_sec || sTimerValue.it_interval.tv_nsec)
		m_bIsRepeat = true;
	else
		m_bIsRepeat = false;

	m_sTimerValue = sTimerValue;

	m_Fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
	
}

CLTimerFd::~CLTimerFd()
{

}

CLStatus CLTimerFd::StartClock()
{
	int ret = timerfd_settime(m_Fd, TFD_TIMER_ABSTIME, &m_sTimerValue, NULL);
	if(-1 == ret)
	{
		CLLogger::WriteLogMsg("In CLMessageReceiverByTimerFd::CLMessageReceiverByTimerFd(), timerfd_settime error", 0);
		return CLStatus(-1, 0);
	}

	return CLStatus(0, 0);
}

CLStatus CLTimerFd::Read()
{
	uint64_t eps;

	ssize_t a = read(m_Fd, &eps, sizeof(uint64_t));
	if(a != sizeof(uint64_t))
	{
		CLLogger::WriteLogMsg("In  CLTimerFd::Read(), read context is not int sizeof unit64_t", 0);
		return CLStatus(-1, 0);
	}
#ifdef debug_for_timer
    printf("timer out!!!!!!!!!!, the value is %ld\n", eps);
#endif 

	return CLStatus(0, 0);
}

const bool& CLTimerFd::IsRepeatTimer()
{
	return m_bIsRepeat;
}
	
const string& CLTimerFd::GetRemoteName()
{
	return m_strRemoteName;
}

const int& CLTimerFd::GetTimerFd()
{
	return m_Fd;
}

const int& CLTimerFd::GetTimerID()
{
	return m_ID;
}


