/*
 * ZipFStream.hpp
 *
 *  Created on: Nov 23, 2017
 *      Author: colinliang
 */

#ifndef LC_IO_ZIPFSTREAM_HPP_
#define LC_IO_ZIPFSTREAM_HPP_

#include "CmdOutputStreamReader.hpp"
#include "../utility/FileUtil.hpp"
#include "../utility/StringUtil.hpp"

namespace LC {
class ZipFStream: public CmdOutputStreamReader {

	ZipFStream(const std::string& cmd, bool cmd_is_filename = false) :
			CmdOutputStreamReader(cmd, cmd_is_filename) {
	}
public:

	static ZipFStream build_istream_from_zip_or_normal_file(const std::string& filename) {
		if (Str::endsWith(Str::toLowerCase(filename), ".zip")) {
			return ZipFStream(std::string("zcat ") + filename);
		} else {
			return ZipFStream(filename, true);
		}
	}

	static void eg_use_ZipFStream(std::string filename = "hello.zip") {
		ZipFStream zfr = build_istream_from_zip_or_normal_file(filename);
		std::istream& is = zfr.get_istream_ref();

		std::string line;
		while (std::getline(is, line)) {
			std::cout << line << std::endl;
		}

		while (std::getline(zfr(), line)) {
			std::cout << line << std::endl;
		}
	}

};
}

#endif /* LC_IO_ZIPFSTREAM_HPP_ */
