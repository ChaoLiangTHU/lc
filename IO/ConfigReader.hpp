/*
 * ConfigReader.hpp
 *
 *  Created on: 2016年8月17日
 *      Author: colinliang
 */

#ifndef CONFIGREADER_HPP_
#define CONFIGREADER_HPP_

#include <string>
#include <map>
#include <fstream>
#include <iostream>

#include "../utility/StringUtil.hpp"
#include "../utility/FileUtil.hpp"
#include "ZipFStream.hpp"
#include <functional>
using std::string;
using std::vector;

namespace LC {

class Key_Value_ConfigReader {
public:
	std::map<string, string> configs;

	std::map<string, string> readConfig(string filename, char splitChar = '\a', char commentChar = '#',
			bool skipfirstline = false, bool verbose = false) {
		std::map<string, string> result;
		std::fstream f(filename.c_str(), std::ios::in);
		if (!f) {
			if (verbose)
				std::cout << "Not valid file: " << filename << std::endl;
			return result;
		}
		string line;

		if (skipfirstline)
			std::getline(f, line);

		if (verbose)
			std::cout << "\n-------- reading config: " << filename << std::endl;
		while (std::getline(f, line)) {
			vector<string> cc = Str::split(line, commentChar);
			line = cc[0];
			Str::trim_inplace(line);

			if (line.size() == 0)
				continue;

			vector<string> kv = Str::split(line, splitChar);
			kv[0] = Str::trim(kv[0]);
			kv[1] = Str::trim(kv[1]);
			result[kv[0]] = kv[1];
//			std::cout << kv[0] << "\t=\t" << kv[1] << endl;
		}
		configs = result;
		if (verbose)
			for (std::map<string, string>::iterator it = configs.begin(); it != configs.end(); ++it)
				std::cout << it->first << "\t=\t" << it->second << std::endl;

		return result;
	}

	std::map<int, int> readInt2IntConfig(string filename, char splitChar = '\a', char commentChar = '#',
			bool skipfirstline = false) {
		std::map<int, int> m;
		readConfig(filename, splitChar, commentChar, skipfirstline);
		for (std::map<string, string>::iterator it = configs.begin(); it != configs.end(); ++it) {
			int k = LC::Private::str2num<int>(it->first.c_str(), (char**) 0);
			int v = LC::Private::str2num<int>(it->second.c_str(), (char**) 0);
			m[k] = v;
		}
		return m;
	}

	template<typename ValueType>
	std::map<string, ValueType> readString2numConfig(string filename, char splitChar = '\a', char commentChar = '#',
			bool skipfirstline = false) {
		std::map<string, ValueType> m;
		readConfig(filename, splitChar, commentChar, skipfirstline);
		for (std::map<string, string>::iterator it = configs.begin(); it != configs.end(); ++it) {
//			std::cout<<it->first<<",  \t"<<it->second<<std::endl;
			ValueType v = LC::Private::str2num<ValueType>(it->second.c_str(), (char**) 0);
			m[it->first] = v;
		}
		return m;
	}

	template<typename T>
	T getNumberValue(string varName, T defaultValue = (T) (0)) {
//		std::cout<<varName<<"\t---\t"<< configs[varName]<<endl;
		if (configs.find(varName) == configs.end()) {
			return defaultValue;
		}
		return LC::Private::str2num<T>(configs[varName].c_str(), (char**) 0);
	}

	string getStringValue(string varName, string defaultValue = "") {
		if (configs.find(varName) == configs.end()) {
			return defaultValue;
		}
		return configs[varName];
	}

	bool hasKey(string key) {
		return configs.find(key) != configs.end();
	}

	string operator[](string varName) {
		return configs[varName];
	}
};

class Vector_ConfigReader { //每行为一个以某个字符分割的配置文件读取类
public:
	template<typename T>
	std::vector<std::vector<T> > readConfig(string filename, char splitChar = '\a', char commentChar = '#',
			bool skipfirstline = false, bool verbose = false) {
		std::vector<std::vector<T> > result;
		std::fstream f(filename.c_str(), std::ios::in);
		if (!f) {
			if (verbose)
				std::cout << "Not valid file: " << filename << std::endl;
			return result;
		}
		string line;

		if (skipfirstline)
			std::getline(f, line);

		if (verbose)
			std::cout << "\n-------- reading config: " << filename << std::endl;
		while (std::getline(f, line)) {
			vector<string> cc = Str::split(line, commentChar);
			line = cc[0];
			Str::trim_inplace(line);

			if (line.size() == 0)
				continue;

			vector<string> values = Str::split(line, splitChar);
			vector<T> vs(values.size());
			for (unsigned int i = 0; i < values.size(); ++i)
				vs[i] = Str::str2num<T>(values[i]);
			result.push_back(vs);
		}
		if (verbose)
			std::cout << "\t" << result.size() << " value lines read." << std::endl;
		return result;
	}

};

class LibSVM_Reader { //每行为一个以某个字符分割的配置文件读取类
public:

	template<typename T>
	static std::pair<std::vector<int>, std::vector<std::vector<T>>> readConfig(string filename, char splitChar = ' ',
			char kvSplitChar = ':', bool skipfirstline = false, bool verbose = false) {
		std::pair<std::vector<int>, std::vector<std::vector<T>>> r;

		std::vector<int>& labels = r.first;
		std::vector<std::vector<T> >& result = r.second;
		std::fstream f(filename.c_str(), std::ios::in);
		if (!f) {
			if (verbose)
			std::cout << "Not valid file: " << filename << std::endl;
			return r;
		}
		string line;

		if (skipfirstline)
		std::getline(f, line);

		if (verbose)
		std::cout << "\n-------- reading libsvm config: " << filename << std::endl;
		while (std::getline(f, line)) {
			Str::trim_inplace(line);
			vector<string> values = Str::split(line, splitChar);
			if (values.size() < 2u)
			continue;

			labels.push_back(Str::str2num<int>(values[0]));

			vector<T> vs(values.size() - 1);
			for (unsigned int i = 1; i < values.size(); ++i) {
				auto kv_str = Str::split(values[i], kvSplitChar);
				if (kv_str.size() != 2u)
				continue;
				int k = Str::str2num<int>(kv_str[0]);
				T v = Str::str2num<T>(kv_str[1]);
				if (vs.size() <= (unsigned int) (k))
				vs.resize(k + 1);
				vs[k] = v;
			}
			result.push_back(vs);
		}
		if (verbose)
		std::cout << "\t" << result.size() << " value lines read." << std::endl;
		return r;
	}

};

/**
 * 类似一个matrix类，其中的元素类型为ElemClass；
 * 输入文件格式： 其中一行为  “0	44:0.7742|1103:0.7658”  其中 0 为该行会被放到矩阵的第几行， 之后为一个以| 分割的values；
 * 具体的value类型， 由ElemClass定义，并由FuncPtr_str2Elem将str（比如44:0.7742）转换为ElemClass类型的数据
 *
 */
template<typename ElemClass = std::pair<int, float> >
class MatrixConfigReader {
public:
//	using FuncPtr_str2PariClass = ElemClass (*)(const std::string&);

	typedef std::function<ElemClass(const std::string&)> FuncPtr_str2ElemClass;

	size_t rows, cols;
	std::vector<ElemClass> data;

	string filename;
	FuncPtr_str2ElemClass funcPtr_str2PariClass;
	char keyVecDelimiter;
	char vecDelimiter;

	MatrixConfigReader(FuncPtr_str2ElemClass funcPtr_str2PariClass = FuncPtr_str2ElemClass(), char keyVecDelimiter =
			'\t', char vecDelimiter = '|') :
			rows(0), cols(0), filename(""), funcPtr_str2PariClass(funcPtr_str2PariClass), keyVecDelimiter(
					keyVecDelimiter), vecDelimiter(vecDelimiter) {

	}

	inline ElemClass* rowPtr(size_t r) {
		if (r >= rows)
			return &data[0];
		return &data[r * cols];
	}

	inline ElemClass& operator()(size_t r, size_t c) {
		return data[r * cols + c];
	}

	/**
	 * 打印该矩阵的第r行，
	 * 注意： 如果ElemClass没法直接输出到ostream，需要自定义一个函数:  std::ostream& operator<< (std::ostream& os, ElemClass kv){os<<"";return os;}
	 * @param r
	 */
	void printRow(size_t r) {
		if (r < rows) {
			std::cout << "[";
			for (size_t c = 0; c < cols; ++c) {
				ElemClass& kv = this->operator ()(r, c);
				std::cout << kv << ", ";
			}
			std::cout << "]" << std::endl;
		}
	}

	size_t determine_cols_in_file(const std::string& filename, int lines_to_check = 5) {
		size_t cols = 0;
//		std::fstream f(filename.c_str(), std::ios::in);
		ZipFStream zfs = ZipFStream::build_istream_from_zip_or_normal_file(filename);
		std::istream& f = zfs.get_istream_ref();

		if (!f) {
			return 0;
		}

		string line;
		int line_count = 0;
		while (std::getline(f, line) && line_count < lines_to_check) {
			Str::trim_inplace(line);
			if (line.size() == 0)
				continue;

			vector<string> k_vec = Str::split(line, keyVecDelimiter);
			if (k_vec.size() != 2)
				continue;
			auto pair_strs = Str::split(k_vec[1], vecDelimiter);

			if (pair_strs.size() > cols)
				cols = pair_strs.size();
		}
//		f.close();
		return cols;

	}

	MatrixConfigReader& readConfig(const std::string& filename, int max_rows, int max_cols = -1, bool verbose = false) {
		if (verbose)
			std::cout << "\n-------- reading config: " << filename << std::endl;

		this->filename = filename;
//		std::ifstream fs(filename.c_str(), std::ios::in);

		ZipFStream zfs = ZipFStream::build_istream_from_zip_or_normal_file(filename);
		std::istream& f = zfs.get_istream_ref();
//		std::istream* pf;
//		if (Str::endsWith(Str::toLowerCase(filename), ".zip") and zfs.operator bool()) {
//			pf=zfs.get_istream_ptr();
//		}else{
//			pf=&fs;
//		}
//		std::istream& f=*pf;

		if (!f) {
			if (verbose)
				std::cout << "Not valid file: " << filename << std::endl;
			return *this;
		}

		rows = max_rows;
		cols = max_cols;

		if (max_cols <= 0) {
			if (verbose)
				std::cout << "max_cols <0, start auto determine_cols_in_file; " << std::endl;

			cols = determine_cols_in_file(filename);

			if (verbose)
				std::cout << "\tcols detected: " << cols << std::endl;
		}

		data.clear();
		if (max_rows <= 0)
			return *this;

		data.resize(rows * cols);

		string line;
		size_t count_invalid_lines = 0;

		while (std::getline(f, line)) {
			Str::trim_inplace(line);

			if (line.size() == 0)
				continue;

			vector<string> values = Str::split(line, keyVecDelimiter);
			if (values.size() != 2) {
				if (verbose) {
					std::cout << "Not valid file: " << filename << std::endl;
					std::cout << "\t	Not valid line: " << line << std::endl;
				}
				continue;
			}

			unsigned int row_idx = Str::str2int(values[0]);
			if (row_idx < rows) {
				auto pair_strs = Str::split(values[1], vecDelimiter);
				unsigned int cols_this_line = pair_strs.size() <= cols ? pair_strs.size() : cols;

				ElemClass* prow = rowPtr(row_idx);
				for (unsigned int i = 0; i < cols_this_line; ++i) {
					prow[i] = funcPtr_str2PariClass(pair_strs[i]);
				}
			} else {
				count_invalid_lines++;
			}

		}

		if (count_invalid_lines > 0 && verbose) {
			std::cout << "# invalid lines (row_idx> max_rows): " << count_invalid_lines << std::endl;
		}

		return *this;
	}
};

//void testMatrixConfigReader() {
//	std::string filename =
//			"/home/colinliang/DATA/Logs/PyCTRDeepLearning/Exp_VideoRecommand/CF_vid/CF_vid_embed_0001_v10_multi_sampling_softmax_loss__chArgFwd/net_model/full_similarity.txt";
//
//	auto str2pair = [](const string& s) {
//		auto kv=LC::Str::split(s,':');
//		if(kv.size()!=2)return std::pair<int,float>(0,0.0f);
//		int k=LC::Str::str2int(kv[0]);
//		float v=LC::Str::str2float32(kv[1]);
//
//		return std::pair<int,float>(k,v);
//	};
//	LC::MatrixConfigReader<std::pair<int, float>> mcr(str2pair);
//	mcr.readConfig(filename, 10, -1, true);
//
//	for (int i = 0; i < 5; ++i)
//		mcr.printRow(i);
//}

}

#endif /* CONFIGREADER_HPP_ */
