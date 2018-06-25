/*
 * CSVReader.hpp
 *
 *  Created on: 2016年8月18日
 *      Author: colinliang
 */

#ifndef CSVREADER_HPP_
#define CSVREADER_HPP_

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <utility>
#include <map>

#include "../utility/StringUtil.hpp"

using std::vector;
using std::string;
using std::map;

using std::cout;
using std::endl;

namespace LC {

class CSVReader {
public:

	///< 读取指定列的内容到vector中，  hasHeadLine为true时将跳过首行
	static vector<vector<string> > readCSV_by_columnIdexes(string filename, vector<int> columnIndexes_toRead,
			char splitChar = ',', bool hasHeadLine = true) {
		vector<vector<string> > results;

		std::fstream f(filename.c_str(), std::ios::in);
		string line;
		long long lineCount = 1;
		if (hasHeadLine) {
			lineCount++;
			bool b = static_cast<bool>(std::getline(f, line));
			if (not b)
				throw string("get header line failed");
		}

		//read data line by line
		while (std::getline(f, line)) {
			lineCount++;
			vector<string> contents = Str::split(line, splitChar);
			if (contents.size() < columnIndexes_toRead.size()) {
				throw string("column number not less than columns to read, line ").append(Str::num2str(lineCount));
			}
			vector<string> r;
			for (std::size_t i = 0; i < columnIndexes_toRead.size(); ++i) {
				r.push_back(contents[columnIndexes_toRead[i]]);
			}
			results.push_back(r);
		}

		f.close();
		return results;
	}

	///< 读取指定列名的内容到vector中，  hasHeadLine为true时将跳过首行
	static vector<vector<string> > readCSV_by_columnNames(string filename, vector<string> columnNames_toRead,
			char splitChar = ',') {
		vector<vector<string> > results;

		std::fstream f(filename.c_str(), std::ios::in);
		string line;
		bool b = static_cast<bool>(std::getline(f, line));
		if (not b)
			throw string("get header line failed");
		vector<string> columnNames_all = Str::split(line, splitChar);
		Str::trim_inplace(columnNames_all);

		// find index of columns to read
		vector<int> idxes;
		for (std::size_t i = 0; i < columnNames_toRead.size(); ++i) {
			for (std::size_t c = 0; c < columnNames_all.size(); ++c) {
				if (columnNames_toRead[i].compare(columnNames_all[c]) == 0) {
					idxes.push_back(c);
					break;
				}
			}
			if (idxes.size() <= i)
				throw string("column not found error: ").append(columnNames_toRead[i]);
		}

		//read data line by line
		long long lineCount = 1;
		while (std::getline(f, line)) {
			lineCount++;
			vector<string> contents = Str::split(line, splitChar);
			if (contents.size() != columnNames_all.size()) {
				throw string("column number not equal to head line, line ").append(Str::num2str(lineCount));
			}
			vector<string> r;
			for (std::size_t i = 0; i < idxes.size(); ++i) {
				r.push_back(contents[idxes[i]]);
			}
			results.push_back(r);
		}
		f.close();

		return results;
	}

	///< 读取文件的所有列的内容到vector中，  hasHeadLine为true时将跳过首行;;如果为false，其实第0行就是column names， 也挺好的
	static vector<vector<string> > readCSV(string filename, char splitChar = ',', bool hasHeadLine = true) {
		vector<vector<string> > results;
		long long lineCount = 0;

		std::fstream f(filename.c_str(), std::ios::in);
		string line;
		if (hasHeadLine) {
			lineCount++;
			bool b = static_cast<bool>(std::getline(f, line));
			if (not b)
				throw string("get header line failed");
		}
		//read data line by line

		while (std::getline(f, line)) {
			lineCount++;
			results.push_back(Str::split(line, splitChar));
		}
		f.close();

		return results;
	}

	///< 统计文件中各个元素的数量，每行有多个元素， 元素之间以splitChar分割;   比如 a,b\na,c  经过count后输出为map{a:2,b:1,c:1}
	static map<string,int> countElements(string filename, char splitChar = ',', bool hasHeadLine = true) {
		map<string,int> results;
		long long lineCount = 0;

		std::fstream f(filename.c_str(), std::ios::in);
		string line;
		if (hasHeadLine) {
			lineCount++;
			bool b = static_cast<bool>(std::getline(f, line));
			if (not b)
				throw string("get header line failed");
		}
		//read data line by line

		while (std::getline(f, line)) {
			lineCount++;
			auto elems=Str::split(line, splitChar);
//			for(const auto& e : elems){
			for(unsigned int i=0;i<elems.size();++i){
				const auto& e=elems[i];
				if(results.find(e)!=results.end())
					results[e]++;
				else
					results[e]=1;
			}
//			results.push_back();
		}
		f.close();

		return results;
	}

	static vector<map<string, string> > readCSV_to_list_of_map(string filename, char splitChar = ',') {
		vector<map<string, string> > r;
		vector<string> columnNames = readColumnNames_of_CSV(filename, splitChar);
		vector<vector<string> > contents = readCSV(filename, splitChar, true);
		for (unsigned int i = 0; i < contents.size(); ++i) {
			map<string, string> m;
			vector<string>& v = contents[i];
			if (v.size() != columnNames.size()) {
				std::stringstream ss;
				ss << "Invalid line " << i << " of file " << filename << endl;
				throw ss.str();
			}
			for (unsigned int c = 0; c < v.size(); ++c) {
				m[columnNames[c]] = v[c];
			}
			r.push_back(m);
		}
		return r;
	}

	static map<string, map<string, string> > readCSV_to_map_of_map(string filename, char splitChar = ',',
			string keyFieldName = "name") {
		map<string, map<string, string> > mm;
		vector<map<string, string> > lm = readCSV_to_list_of_map(filename, splitChar);
		for (unsigned int i = 0; i < lm.size(); ++i) {
			if (lm[i].find(keyFieldName) == lm[i].end()) {
				cout << "No key field" << keyFieldName << " in line " << i << " of file" << filename << endl;
				throw "No key field";
			}

			mm[lm[i][keyFieldName]] = lm[i];
		}
		return mm;
	}

	///<  读取CSV文件的列名，，要求文件必须是有列名行的！！！！！
	static vector<string> readColumnNames_of_CSV(string filename, const char splitChar = ',') {
		std::fstream f(filename.c_str(), std::ios::in);
		string line;
		bool b = static_cast<bool>(std::getline(f, line));
		if (not b)
			throw string("get header line failed");
		vector<string> columnNames_all = Str::split(line, splitChar);
		Str::trim_inplace(columnNames_all);
		f.close();
		return columnNames_all;
	}

	///< 读取CSV配置文件，其中第一行为全部列的名称，第二行为模型要用到的字段的名称，该方法用于生成readCSV_by_columnIdexes()所需的列标
	static vector<int> parseColumnIndexesToRead(string configFilename, char splitChar = ',') {
		std::fstream f(configFilename.c_str(), std::ios::in);
		string line;
		bool b;

		// 第一行，全部列名
		b = static_cast<bool>(std::getline(f, line));
		if (not b)
			throw string("get header line failed");
		vector<string> columnNames_all = Str::split(line, splitChar);
		Str::trim_inplace(columnNames_all);

		// 第二行，需要读取的列名
		b = static_cast<bool>(std::getline(f, line));
		if (not b)
			throw string("get sencond line failed");
		vector<string> columnNames_toRead = Str::split(line, splitChar);
		Str::trim_inplace(columnNames_toRead);

		// find index of columns to read
		vector<int> idxes;
		for (std::size_t i = 0; i < columnNames_toRead.size(); ++i) {
			for (std::size_t c = 0; c < columnNames_all.size(); ++c) {
				if (columnNames_toRead[i].compare(columnNames_all[c]) == 0) {
					idxes.push_back(c);
					break;
				}
			}
			if (idxes.size() <= i)
				throw string("column not found error: ").append(columnNames_toRead[i]);
		}

		return idxes;
	}
};

}

#endif /* CSVREADER_HPP_ */
