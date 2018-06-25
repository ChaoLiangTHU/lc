/*
 * FileUtil.hpp
 *
 *  Created on: Dec 20, 2016
 *      Author: colinliang
 */

#ifndef LC_UTILITY_FILEUTIL_HPP_
#define LC_UTILITY_FILEUTIL_HPP_
#include <string>
#include <vector>
#include <utility>
#include <cstdlib>
#include <fstream>
#include "third_party/tinydir.h" //https://github.com/cxong/tinydir

using std::string;
using std::pair;
using std::vector;
#include "StringUtil.hpp"
namespace LC {
class Directory {
public:
	static string basename(const string& path) {
		string::size_type lastSlash = path.find_last_of("/\\");
		if (lastSlash == std::string::npos) {
			return path;
		}
		return path.substr(lastSlash + 1);
	}

	static string dirname(const string& path) {
		auto lastSlash = path.find_last_of("/\\");
		if (lastSlash == std::string::npos) {
			return path;
		}
		return path.substr(0, lastSlash);
	}

	static string join(const string& path, const string& name) {
		if (path.size() == 0)
			return name;
		char last = path[path.size() - 1];
		if (last == '\\' || last == '/') {
			return path + name;
		} else {
			return path+ "/" + name;
		}
	}

	static pair<vector<string>, vector<string> > listFilesAndDirs(const string& dir_name) { ///< 返回文件列表和 子目录列表(只包含在dir_name下的文件/路径名，不包含dir_name)； 另一种方法时使用linux的glob.h ， 或者C++ 17的filesystem
		//参见： http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
		tinydir_dir dir;
		tinydir_open(&dir, dir_name.c_str());
		pair<vector<string>, vector<string> > f_d;
		vector<string>& filess = f_d.first;
		vector<string>& dirs = f_d.second;

		while (dir.has_next) {
			tinydir_file f;
			tinydir_readfile(&dir, &f);
//			std::cout<<f.name<<std::endl;
			if (f.is_dir) {
				dirs.push_back(f.name);
			} else {
				filess.push_back(f.name);
			}
			tinydir_next(&dir);
		}
		tinydir_close(&dir);
		return f_d;
	}

	static pair<vector<string>, vector<string> > listFilesAndDirs_fullpath(const string& dir_name) { ///< 返回文件列表和 子目录列表(绝对路径)； 另一种方法时使用linux的glob.h ， 或者C++ 17的filesystem
		pair<vector<string>, vector<string> > f_d = listFilesAndDirs(dir_name);
		vector<string>& fs = f_d.first;
		vector<string>& ds = f_d.second;
		for (unsigned int i = 0; i < fs.size(); ++i) {
			fs[i] = join(dir_name,fs[i]);
		}
		for (unsigned int i = 0; i < ds.size(); ++i) {
			ds[i] = join(dir_name,ds[i]);
		}
		return f_d;
	}
	static vector<string> listFiles(const string& dir_name) { ///< 返回文件列表和 子目录列表(只包含在dir_name下的文件/路径名，不包含dir_name)； 另一种方法时使用linux的glob.h ， 或者C++ 17的filesystem
		//参见： http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
		tinydir_dir dir;
		tinydir_open(&dir, dir_name.c_str());
		vector<string> filess;

		while (dir.has_next) {
			tinydir_file f;
			tinydir_readfile(&dir, &f);
//			std::cout<<f.name<<std::endl;
			if (f.is_dir) {
				;
			} else {
				filess.push_back(f.name);
			}
			tinydir_next(&dir);
		}
		tinydir_close(&dir);
		return filess;
	}

	static vector<string> listFiles_fullpath(const string& dir_name) { ///< 返回文件列表和 子目录列表(绝对路径)； 另一种方法时使用linux的glob.h ， 或者C++ 17的filesystem
		vector<string> fs = listFiles(dir_name);
		for (unsigned int i = 0; i < fs.size(); ++i) {
			fs[i] = join(dir_name,fs[i]);
		}
		return fs;
	}

	static vector<string> listFiles_startswith_fullpath(const string& dir_name, string prefix) { ///< 返回文件列表和 子目录列表(绝对路径)； 另一种方法时使用linux的glob.h ， 或者C++ 17的filesystem
		vector<string> fs = listFiles(dir_name);
		vector<string> fsfiltered;
		for (unsigned int i = 0; i < fs.size(); ++i) {
			if (LC::Str::startsWith(fs[i], prefix))
				fsfiltered.push_back(join(dir_name,fs[i]));
		}
		return fsfiltered;
	}

	static vector<string> listDirs(const string& dir_name) { ///< 返回文件列表和 子目录列表(只包含在dir_name下的文件/路径名，不包含dir_name)； 另一种方法时使用linux的glob.h ， 或者C++ 17的filesystem
		//参见： http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
		tinydir_dir dir;
		tinydir_open(&dir, dir_name.c_str());
		vector<string> dirs;

		while (dir.has_next) {
			tinydir_file f;
			tinydir_readfile(&dir, &f);
//			std::cout<<f.name<<std::endl;
			if (f.is_dir) {
				dirs.push_back(f.name);
			}
			tinydir_next(&dir);
		}
		tinydir_close(&dir);
		return dirs;
	}

	static vector<string> listDirs_fullpath(const string& dir_name) { ///< 返回文件列表和 子目录列表(绝对路径)； 另一种方法时使用linux的glob.h ， 或者C++ 17的filesystem
		vector<string> ds = listDirs(dir_name);
		for (unsigned int i = 0; i < ds.size(); ++i) {
			ds[i] = join(dir_name,ds[i]);
		}
		return ds;
	}
	static vector<string> listDirs_startswith_fullpath(const string& dir_name, const string& prefix) { ///< 返回文件列表和 子目录列表(绝对路径)； 另一种方法时使用linux的glob.h ， 或者C++ 17的filesystem
		vector<string> ds = listDirs(dir_name);
		vector<string> dsf;
		for (unsigned int i = 0; i < ds.size(); ++i) {
			if (LC::Str::startsWith(ds[i], prefix)) {
				dsf.push_back(join(dir_name,ds[i]));
			}
		}
		return dsf;
	}



	static bool isfile(const string& filename) {
		std::fstream f;
		f.open(filename.c_str(), std::ios::in);
		if (!f) {
			f.close();
			return false;
		} else {
			f.close();
			return true;
		}
	}
	static bool isdir(const string& dir_name) {
		if (dir_name.compare("/") == 0)
			return true;

		string name = basename(dir_name);
		string parent = dirname(dir_name);
		auto f_d = listFilesAndDirs(parent);
		vector<string>& ds = f_d.second;
		for (unsigned int i = 0; i < ds.size(); ++i) {
			if (name.compare(ds[i]) == 0)
				return true;
		}
		return false;
	}

	static int rm(const string& dir_or_file) {
		string cmd = string("rm -f -r \"") + dir_or_file + "\"";
		int ret = std::system(cmd.c_str());
		return ret;
	}

};

class ZipUtil {
public:
	static int unzip_file(const string& zipfile, const string& dstdir = "./net_model/", bool override_output = true) {
		if (zipfile.length() == 0) {
			return 1;
		}
		string zipFileName = zipfile;
		int return_unzip;
		if (override_output)
			return_unzip = std::system(string("unzip -o ").append(zipFileName).append(" -d ").append(dstdir).c_str()); //unzip configs, -o to override
		else
			return_unzip = std::system(string("unzip ").append(zipFileName).append(" -d ").append(dstdir).c_str()); //unzip configs, -o to override
		return return_unzip;
	}
};

}

#endif /* LC_UTILITY_FILEUTIL_HPP_ */
