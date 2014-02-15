#include <string.h>
#include <vector>
#include <unistd.h>
#include <errno.h>
#include <string>
#include "CLProcessFunctionForExec.h"
#include "CLLogger.h"
#include "ErrorCode.h"

using namespace std;

#define MAX_LENGTH_OF_PATH 1024

CLProcessFunctionForExec::CLProcessFunctionForExec()
{
}

CLProcessFunctionForExec::~CLProcessFunctionForExec()
{
}

CLStatus CLProcessFunctionForExec::RunExecutiveFunction(void* pCmdLine)
{
	int len = strlen((char *)pCmdLine);

	char *pstrCmdLine = new char[len + 1];
	strcpy(pstrCmdLine, (char *)pCmdLine);

	char *p = pstrCmdLine;

	vector<char *> vstrArgs;

	while(char *q = strsep(&p, " "))
	{
		if(*q == 0)
			continue;

		vstrArgs.push_back(q);
	}

	char **argv = new char* [vstrArgs.size() + 1];
	for(int i = 0; i < vstrArgs.size(); i++)
		argv[i] = vstrArgs[i];
	argv[vstrArgs.size()] = NULL;

	try
	{
		char old_directory[MAX_LENGTH_OF_PATH];
		if(getcwd(old_directory, MAX_LENGTH_OF_PATH) == 0)
		{
			CLLogger::WriteLogDirectly("In CLProcessFunctionForExec::RunExecutiveFunction(), getcwd error", 0);
			throw CLStatus(-1, NORMAL_ERROR);
		}

		if(!SetWorkDirectory(argv[0]).IsSuccess())
			throw CLStatus(-1, NORMAL_ERROR);

		execv(argv[0], argv);

		if(chdir(old_directory) == -1)
			CLLogger::WriteLogDirectly("In CLProcessFunctionForExec::RunExecutiveFunction(), chdir error", 0);

		CLLogger::WriteLogDirectly("In CLProcessFunctionForExec::RunExecutiveFunction(), execv error", errno);

		throw CLStatus(-1, NORMAL_ERROR);
	}
	catch(CLStatus& s)
	{
		delete [] argv;
		delete [] pstrCmdLine;

		return s;
	}
}

CLStatus CLProcessFunctionForExec::SetWorkDirectory(char *pstrArgv0)
{
	string str = pstrArgv0;

	size_t pos = str.rfind("/");
	if(pos == -1)
		return CLStatus(0, 0);

	string strDirectory = str.substr(0, pos + 1);

	if(str.length() == pos + 1)
	{
		CLLogger::WriteLogDirectly("In CLProcessFunctionForExec::SetWorkDirectory(), argv0 error", 0);
		return CLStatus(-1, NORMAL_ERROR);
	}

	string strFileName = str.substr(pos + 1);
	strcpy(pstrArgv0,strFileName.c_str());

	if(chdir(strDirectory.c_str()) == -1)
	{
		CLLogger::WriteLogDirectly("In CLProcessFunctionForExec::SetWorkDirectory(), chdir error", 0);
		return CLStatus(-1, NORMAL_ERROR);
	}

	return CLStatus(0, 0);
}