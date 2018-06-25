/*
 * GeneralArgParser.hpp
 *
 *  Created on: Jun 25, 2018
 *      Author: colinliang
 */

#ifndef LC_UTILITY_GENERALARGPARSER_HPP_
#define LC_UTILITY_GENERALARGPARSER_HPP_

#include <unordered_map>
#include <stdexcept>
#include "StringUtil.hpp"

namespace LC {
class GeneralArgParser {
public:
	std::unordered_map<std::string, std::string> args;
	GeneralArgParser() {
	}

	GeneralArgParser(int argc, char** argv) {
		for (int i = 0; i < argc; ++i) {
			std::string s = argv[i];
			auto kv = Str::split(s, '=');
			if (kv.size() == 1 and i == 0)
				continue;
			if (kv.size() > 2)
				throw std::invalid_argument(s);
			args[kv[0]] = kv[1];

		}
	}

	template<typename ValType>
	ValType get(const std::string& key, const ValType& default_val) {
		auto it = args.find(key);
		if (it != args.end())
			return Str::str2num<ValType>(it->second);
		return default_val;
	}


	std::string getstr(const std::string& key, const std::string& default_val) {
		auto it = args.find(key);
		if (it != args.end())
			return it->second;
		return default_val;
	}

	std::string toString() {
		std::stringstream ss;
		ss << "---Arguments: " << std::endl;
		for (const auto& kv : args)
			ss << "\t" << kv.first << " = " << kv.second << std::endl;
		ss << "---End of Arguments";
		return ss.str();
	}
};
}

#endif /* LC_UTILITY_GENERALARGPARSER_HPP_ */
