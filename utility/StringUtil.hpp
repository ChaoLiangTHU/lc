/*
 * StringUtil.hpp
 *
 *  Created on: 2016年8月18日
 *      Author: colinliang
 */

#ifndef STRINGUTIL_HPP_
#define STRINGUTIL_HPP_
//STL
#include <limits>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <functional>
#include <algorithm>

//string
#include <cstring>
#include <string>
#include <sstream>

//file
#include <fstream>

#include <iostream>

using std::string;
using std::vector;
using std::cout;
using std::endl;

namespace LC {

namespace Private {
template<typename T>
inline T str2num(const char* p, char** pend) {
	return static_cast<T>(std::strtod(p, pend));
}

template<>
inline string str2num(const char* p, char** pend) {
	//cout<<"float~~";
	return std::string(p);
}

template<>
inline char str2num(const char* p, char** pend) {
	//cout<<"float~~";
	return p[0];
}

template<>
inline float str2num(const char* p, char** pend) {
	//cout<<"float~~";
	return std::strtof(p, pend);
}

template<>
inline int str2num(const char* p, char** pend) {
	//cout<<"int ~~";
	return std::strtol(p, pend, 10);
}

template<>
inline long int str2num(const char* p, char** pend) {
	//cout<<"long int ~~";
	return std::strtol(p, pend, 10);
}

template<>
inline long long int str2num(const char* p, char** pend) {
	return std::strtoll(p, pend, 10);
}

}

///////////////////////////////////////////////////////

class Str {

private:
	static int find_utf8_start_char_less(const char* pch, int pos) {
		if (pos <= 0)
			return 0;
		while (pos >= 0) {
			//		cout << ch[pos] << ": " << (int) (*((unsigned char*) (ch + pos))) << endl;
			unsigned char ch = pch[pos];
			if (ch < (unsigned char) (0x80) || ch >= (unsigned char) (0xC0)) //https://en.wikipedia.org/wiki/UTF-8
				return pos;
			else
				pos--;
		}
		if (pos < 0)
			pos = 0;
		return pos;
	}
public:
	/**
	 * 将 utf-8编码的字符串 truancate到 最多size_in_byte个字符。 该方法会保证截断后的字符串是有效的
	 * @param str
	 * @param size_in_byte
	 * @return
	 */
	static std::string utf8_str_truncate(const std::string& str, size_t size_in_byte) {
		if (size_in_byte >= str.size())
			return str.substr(0, str.size());
		int pos = find_utf8_start_char_less(str.data(), size_in_byte);
		//	cout << "expected size: " << size_in_byte << "; \ttrue truncated size: " << pos << endl;
		return str.substr(0, pos);
	}

private:

//	///< copied from http://stackoverflow.com/questions/236129/split-a-string-in-c
//	static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
//		std::stringstream ss(s);
//		std::string item;
//		while (std::getline(ss, item, delim)) {
//			elems.push_back(item);
//		}
//		return elems;
//	}
public:
//	///< http://stackoverflow.com/questions/236129/split-a-string-in-c
//	static std::vector<std::string> split(const std::string &s, char delim) {
//		std::vector<std::string> elems;
//		split(s, delim, elems);
//		return elems;
//	}

///< http://blog.csdn.net/butterfly_dreaming/article/details/10142443
	static std::vector<std::string> split(const std::string& s, const char delim) {
		std::vector<std::string> ret;
		size_t last = 0;
		size_t index = s.find_first_of(delim, last);
		while (index != std::string::npos) {
			ret.push_back(s.substr(last, index - last));
			last = index + 1;
			index = s.find_first_of(delim, last);
		}

		ret.push_back(s.substr(last));

		return ret;
	}

	///< http://blog.csdn.net/butterfly_dreaming/article/details/10142443
	static std::vector<std::string> split(const std::string& s, const std::string& delim) {
		std::vector<std::string> ret;
		size_t last = 0;
		size_t index = s.find_first_of(delim, last);
		while (index != std::string::npos) {
			ret.push_back(s.substr(last, index - last));
			last = index + 1;
			index = s.find_first_of(delim, last);
		}

		ret.push_back(s.substr(last));

		return ret;
	}

	///<   http://blog.csdn.net/butterfly_dreaming/article/details/10142443
	static std::string trim(const std::string &s, std::string eliminators = " \t\n") {
		std::string r = s;
		if (r.empty()) {
			return r;
		}
		r.erase(0, r.find_first_not_of(eliminators));
		r.erase(r.find_last_not_of(eliminators) + 1);
		return r;
	}

	static std::vector<std::string> trim(const std::vector<std::string>& vs, std::string eliminators = " \t\n") {
		std::vector<std::string> rs;
		for (unsigned int i = 0; i < vs.size(); ++i)
			rs.push_back(trim(vs[i]));
		return rs;
	}

	static std::string& trim_inplace(std::string &s, std::string eliminators = " \t\n") {
		if (s.empty()) {
			return s;
		}
		s.erase(0, s.find_first_not_of(eliminators));
		s.erase(s.find_last_not_of(eliminators) + 1);
		return s;
	}

	static std::vector<std::string>& trim_inplace(std::vector<std::string>& vs, std::string eliminators = " \t\n") {
		for (unsigned int i = 0; i < vs.size(); ++i)
			trim_inplace(vs[i]);
		return vs;
	}

	static std::string& replace_inplace(std::string& s, char from, char to) { //https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
		std::replace(s.begin(), s.end(), from, to);
		return s;
	}
	static std::string replace(const std::string& s, char from, char to) { //https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
		std::string r = s.c_str(); //https://stackoverflow.com/questions/4751446/alternative-of-strcpy-in-c
		std::replace(r.begin(), r.end(), from, to);
		return r;
	}
	/**
	 * 字符串替换，注意传入的不是引用，因而是新建了一个字符串
	 * @param str
	 * @param from
	 * @param to
	 * @return
	 */
	std::string replace(const std::string& s, const std::string& from, const std::string& to) { //https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
		std::string r = s.c_str(); //https://stackoverflow.com/questions/4751446/alternative-of-strcpy-in-c
		size_t start_pos = 0;
		while ((start_pos = r.find(from, start_pos)) != std::string::npos) {
			r.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
		return r;
	}

	static void printStrVec(const vector<string>& vs) {
		cout << "vector of string (size=" << vs.size() << "): " << endl;
		for (unsigned int i = 0; i < vs.size(); ++i) {
			cout << "\tlen=" << vs[i].size() << ":\t" << vs[i] << endl;
		}
	}

	template<typename T>
	static string vector2string(const vector<T>& vec, string splitStr = ", ") {
		std::stringstream ss;
		ss << "[";
		for (unsigned int i = 0; i < vec.size(); ++i) {
			ss << vec[i] << splitStr;
		}
		ss << "]";
		return ss.str();
	}

	template<typename T>
	static inline std::string vec2str(const vector<T>& vec, string splitStr = ", ") {
		return vector2string(vec, splitStr);
	}

	template<typename K, typename V>
	static string vectpair2string(const vector<std::pair<K, V>>& vec, string delimiter = "; ", string kv_delimiter =
			", ") {
		std::stringstream ss;
		ss << "[";
		for (unsigned int i = 0; i < vec.size(); ++i) {
			ss << "(" << vec[i].first << kv_delimiter << vec[i].second << ")" << delimiter;
		}
		ss << "]";
		return ss.str();
	}

//	template<typename T>
//	static T copy_noUse(const T& t) {
//		return t;
//	}
	static std::string toLowerCase(const std::string& data) {
		std::string ret;
		ret.resize(data.size());
		for (unsigned int i = 0; i < ret.size(); ++i)
			ret[i] = std::tolower(data[i]);
		return ret;
	}

	static bool startsWith(const std::string& str, const std::string& sub) {
//		return str.size() >= sub.size()
//				&& str.substr(0, sub.size()).compare(sub) == 0;
		return str.rfind(sub, 0) == 0u;
	}

	static bool endsWith(const std::string& str, const std::string& sub) {
//		return str.size() >= sub.size()
//				&& str.substr(str.size() - sub.size(), sub.size()).compare(sub)
//						== 0;
		return str.size() >= sub.size() && str.find(sub, str.size() - sub.size()) == 0u;
	}

	static bool contains(const std::string& str, const std::string& sub) {
		return str.find(sub) != std::string::npos;
	}

	//--------------------------------  convertions
	inline static float str2float32(const char* p, char** pend = (char**) 0) {
		//cout<<"float~~";
		return std::strtof(p, pend);
	}

	inline static float str2float32(const string& p, char** pend = (char**) 0) {
		return std::strtof(p.c_str(), pend);
	}

	inline static float str2double(const char* p, char** pend = (char**) 0) {
		//cout<<"float~~";
		return std::strtod(p, pend);
	}

	inline static float str2double(const string& p, char** pend = (char**) 0) {
		return std::strtod(p.c_str(), pend);
	}

	inline static int str2int(const char* p, char** pend = (char**) 0) {
		//cout<<"int ~~";
		return std::strtol(p, pend, 10);
	}

	inline static int str2int(const string& p, char** pend = (char**) 0) {
		//cout<<"int ~~";
		return std::strtol(p.c_str(), pend, 10);
	}

	inline static short str2short(const char* p, char** pend = (char**) 0) {
		//cout<<"int ~~";
		return std::strtol(p, pend, 10);
	}

	inline static short str2short(const string& p, char** pend = (char**) 0) {
		//cout<<"int ~~";
		return std::strtol(p.c_str(), pend, 10);
	}

	inline static long int str2long(const char* p, char** pend = (char**) 0) {
		//cout<<"long int ~~";
		return std::strtol(p, pend, 10);
	}

	inline static long int str2long(const string& p, char** pend = (char**) 0) {
		//cout<<"long int ~~";
		return std::strtol(p.c_str(), pend, 10);
	}

	inline static long long int str2longlong(const char* p, char** pend = (char**) 0) {
		return std::strtoll(p, pend, 10);
	}

	inline static long long int str2longlong(const string& p, char** pend = (char**) 0) {
		return std::strtoll(p.c_str(), pend, 10);
	}

	template<typename T>
	inline static T str2num(const char* p, char** pend = (char**) 0) {
		return LC::Private::str2num<T>(p, pend);
	}

	template<typename T>
	inline static T str2num(const string& str) {
		return LC::Private::str2num<T>(str.data(), (char**) 0);
	}

	template<typename T>
	static std::vector<T> str2numVec(const char* p, int len = -1) {
		std::vector<T> ds;
		len = len > 0 ? len : std::strlen(p);
		const char* const pend = p + len;
		char *end;
//		std::cout << "Parsing '" << p << "':\n";

		while (p < pend) {
//			std::cout << "\tsub Parsing '" << p << "':\n";
			T d = str2num<T>(p, &end);
//			std::cout<<"\t\t"<<d<<std::endl;
			if (p == end)
				return ds;
//					if (errno == ERANGE) {
//						throw("range error, got ");
//						errno = 0;
//					}
			ds.push_back(d);
			p = end + 1;
		}
		return ds;
	}

	/**
	 * 与str2numVec 不同， 该函数解析内容不包括 INF，NAN， 0x开头的数等；
	 * 但是可以支持解析存在其他字母的情况，比如3:[f1<19.5] yes=7,no=8,missing=-7 会解析出 3,1,19.5,7,8,-7
	 */
	static std::vector<double> str2doublevec_ignore_letters(const char* p, int len = -1) {
		std::vector<double> ds;
		len = len > 0 ? len : std::strlen(p);
		const char* const pend = p + len;
		char *end;
		//std::cout << "Parsing '" << p << "':\n";

		while (p < pend) {
			while (*p != '-' && (*p < '0' || *p > '9')) {
				p++;
				if (p >= pend)
					return ds;
			}
			double d = std::strtod(p, &end);
			if (p == end)
				return ds;
			//		if (errno == ERANGE) {
			//			throw("range error, got ");
			//			errno = 0;
			//		}
			ds.push_back(d);
			p = end + 1;
		}
		return ds;
	}

	template<typename T>
	inline static std::vector<T> str2numVec(string str) {
		return str2numVec<T>(str.c_str(), str.size());

	}

//	template<typename T>   // 无法找到该方法。。。。好奇怪
//	static std::string num2str(T t) {
//		std::stringstream ss;
//		ss << t;
//		return ss.str();
//	}
	/**
	 #define __num2strfunc__(type)	static std::string num2str(type t) {\
		std::stringstream ss;\
		ss << t;\
		return ss.str();\
	}
	 */
	//gcc 4.4.2上无法编译
#define __num2strfunc__(type)	inline static std::string num2str(type t) {	return std::to_string(t);}

	__num2strfunc__(int)__num2strfunc__(long)
	__num2strfunc__(long long)
	__num2strfunc__(float)
	__num2strfunc__(double)
	__num2strfunc__(unsigned int)
	__num2strfunc__(unsigned long long)

	template<typename T>
	static std::vector<T> strVec2NumVec_SupportInf(std::vector<std::string> strs) {
		std::vector<T> ret(strs.size());
		for (unsigned int i = 0; i < strs.size(); ++i) {
			std::string str = trim(strs[i]);
			str = toLowerCase(str);
			if (str.compare("inf") == 0) {
				if (std::numeric_limits<T>::has_infinity)
				ret[i] = std::numeric_limits<T>::infinity();
				else
				ret[i] = std::numeric_limits<T>::max();
			} else if (str.compare("-inf") == 0) {
				if (std::numeric_limits<T>::has_infinity)
				ret[i] = -std::numeric_limits<T>::infinity();
				else
				ret[i] = std::numeric_limits<T>::min();
			} else {
				ret[i] = Private::str2num<T>(str.data(), (char**) (0));
			}
		}
		return ret;
	}
};

template<>
inline string Str::str2num<string>(const string& str) {
	return str;
}

}

#endif /* STRINGUTIL_HPP_ */
