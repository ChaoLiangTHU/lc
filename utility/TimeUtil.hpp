#pragma once

//#include <time.h>
#include <ctime>
#include <iomanip>
#include <stdexcept>
#include <sstream>

namespace LC {
class TimeUtil {
public:
	static long long getCurTime_second() {
		return std::time(0);
	}

	static inline std::time_t now_second() {
		return std::time(nullptr);
	}

	static inline std::tm getTm(const std::time_t time = now_second()) {
		std::tm t;
		std::tm* pt = std::localtime(&time);
		t = *pt;
		return t;
	}

	static inline std::tm zone0timeStructure(const std::time_t time = now_second()) {
		std::tm t;
		std::tm* pt = std::localtime(&time);
		t = *pt;
		return t;
	}

	static inline std::tm getTm(const std::string& timestr, const std::string& format = "%Y-%m-%d %H:%M:%S") {
		std::tm t = { };
		std::istringstream ss(timestr);
//		ss.imbue(std::locale("de_DE.utf-8"));
		ss >> std::get_time(&t, format.c_str());
		if (ss.fail()) {
			throw std::invalid_argument(std::string("ERROR: convert time failed: ") + timestr + ";  format=" + format);
		}

//		std::strftime()
		return t;
	}

	static inline std::time_t tm2time_t(std::tm& t){
		return std::mktime(&t);
	}

	static inline std::time_t getTime_t(const std::string& timestr, const std::string& format = "%Y-%m-%d %H:%M:%S") {
		std::tm t = getTm(timestr, format);
		return std::mktime(&t);
	}

	static inline std::string tm2str(const std::tm& t) {
		std::stringstream ss;
		ss << std::put_time(&t, "%c");
		return ss.str();
	}

};

}
