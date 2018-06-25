/*
 * logUtil.hpp
 *
 *  Created on: May 11, 2017
 *      Author: colinliang
 */

#ifndef LC_UTILITY_LOGUTIL_HPP_
#define LC_UTILITY_LOGUTIL_HPP_
#include <sstream>

#ifndef LCLogf
//日志，类似于printf之类的方法，默认采用vprintf实现可变参数个数的日志；
//切换到其他日志系统的话，可以采用类似#define LCLogf(...)  do{LC::Logf::logf(__VA_ARGS__);}while(0)的方法
//也可以通过 #define LCLogf(...)  do{;}while(0)  来屏蔽日志
#include <stdarg.h>
#include <stdio.h>
namespace LC {
class Logf {
public:
	static void logf(const char* szFormat, ...) {
		va_list ap;
		va_start(ap, szFormat);
		vprintf(szFormat, ap);
		va_end(ap);

	}

//	template<typename T>
//	void logf(const T& t) {
//		std::stringstream ss;
//		ss << t;
//		logf("%s", ss.str().c_str());
//
//	}
//
//	static void logf(const string& s) {
//		logf("%s", s.c_str());
//	}

};
}

#define LCLogf(...)  do{LC::Logf::logf(__VA_ARGS__);}while(0)

#endif

#define lclogf(...)  do{LCLogf(__VA_ARGS__);}while(0)
#define LCLog(exp) do{std::stringstream __ss;__ss<<(exp);lclogf("%s",__ss.str().c_str());}while(0)

#define lclogv(exp) LCLog(exp)
#define lclogn(exp) lclogf(#exp)
#define lclognv(exp) do{std::stringstream __ss;__ss<<(exp);lclogf("%s: %s",#exp,__ss.str().c_str());}while(0) //  LCLog(std::string(#exp)+": ");LCLog(exp)

#ifndef lclogl
#define lclogl() lclogf("\n")
#endif
#define lclogfl(...) do{lclogf(__VA_ARGS__);lclogl();}while(0)

#define lclogvl(exp) do{lclogv(exp); lclogl();}while(0)
#define lclognl(exp) do{lclogn(exp);lclogl();}while(0)
#define lclognvl(exp) do{lclognv(exp); lclogl();}while(0)

#define lclogpos() lclogf("FUNC: %s(), \tLINE: %d, \tFILE: %s\n",__FUNCTION__,__LINE__,__FILE__)
#endif /* LC_UTILITY_LOGUTIL_HPP_ */
