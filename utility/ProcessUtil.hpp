/*
 * ProcessUtil.hpp
 *
 *  Created on: Jan 9, 2018
 *      Author: colinliang
 */

#ifndef LC_UTILITY_PROCESSUTIL_HPP_
#define LC_UTILITY_PROCESSUTIL_HPP_

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace LC {

class ProcessUtil {
public:
	static int getpid() {
		return ::getpid();
	}

	static int getppid() {
		return ::getppid();
	}
};

}

#endif /* LC_UTILITY_PROCESSUTIL_HPP_ */
