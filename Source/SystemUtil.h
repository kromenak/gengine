//
// SystemUtil.h
//
// Clark Kromenaker
//
// Some cross-platform helper functions for querying the system.
//
#pragma once
#include <string>
#include <unistd.h>
#include <limits.h>

namespace SystemUtil
{
	inline std::string GetComputerName()
	{
		//TODO: Implement for Windows.
		char computerName[_POSIX_HOST_NAME_MAX];
		gethostname(computerName, _POSIX_HOST_NAME_MAX);
		return std::string(computerName);
	}
	
	inline std::string GetUserName()
	{
		//TODO: Implement for Windows.
		char userName[_POSIX_LOGIN_NAME_MAX];
		getlogin_r(userName, _POSIX_LOGIN_NAME_MAX);
		return std::string(userName);
	}
}
