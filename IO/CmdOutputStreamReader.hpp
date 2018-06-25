#pragma once
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include "../utility/third_party/fdstream.hpp"
#include "../utility/ReferenceCounter.hpp"

namespace LC {
/**
 * 执行cmd 命令，并从其标准输出中读取数据
 * 注意： 要保证该类的get_istream_ref()所返回的std::istream 有效， 则必须保证该类的实例没有被析构
 * istream继承关系： http://www.cplusplus.com/reference/istream/istream/
 * 通过boost库将FILE* 转换为istream : http://www.cplusplus.com/forum/general/16532/
 * popen 相关：https://www.cnblogs.com/caosiyang/archive/2012/06/25/2560976.html
 * 使用zlib 读取 zip文件相关：https://stackoverflow.com/questions/3201538/how-to-read-a-gz-file-line-by-line-in-c
 */
class CmdOutputStreamReader {
	FILE* fp;  // FILE* 只能配合fgets函数使用，比较麻烦，这里将其通过第三方库转为istream类型
	boost::fdinbuf* pbuf;
	std::istream* pis;
	ReferenceCounter rc;
public:
	CmdOutputStreamReader(const std::string& cmd, bool cmd_is_filename = false) :
			fp(NULL), pbuf(NULL), pis(NULL) {
		if (cmd_is_filename) {
			pis=new std::ifstream(cmd.c_str(), std::ios::in);
		} else {
			fp = ::popen(cmd.c_str(), "r"); //建立管道
			if (fp) {
				pbuf = new boost::fdinbuf(fileno(fp));
				pis = new std::istream(NULL);
				pis->rdbuf(pbuf);
			}
		}

	}
	~CmdOutputStreamReader() {
		if (rc.needToRelease()) {
			if (pis != NULL) {
//				std::cout<<"delete istream"<<std::endl;
				delete pis;
				pis = NULL;
			}
			if (pbuf != NULL) {
//				std::cout<<"delete buffer allocated for popen"<<std::endl;
				delete pbuf;
				pbuf = NULL;
			}

			if (fp) {
//				std::cout<<"delete file pointer opened by popen"<<std::endl;
				pclose(fp);
				fp = NULL;
			}
		}
	}
	inline FILE* get_file_ptr() {
		return fp;
	}
	inline int get_file_number() {
		return fileno(fp);
	}

	inline std::istream* get_istream_ptr() {
		return pis;
	}

	inline std::istream& get_istream_ref() {
		return *pis;
	}

//	inline operator std::istream&(){ //不生效。。。。
//		return *pis;
//	}

	inline std::istream& operator()() {
		return *pis;
	}

	inline  operator bool() {
		if (pis == NULL) {
			return false;
		}

		return pis;
	}
	/**
	 * 从 zip 文件中获取输入流（该输入流包含在CmdOutputStreamReader中）
	 * @param filename
	 * @return
	 */
	static CmdOutputStreamReader readZipFile(const std::string& filename) {
		std::string cmd = std::string("zcat ") + filename;
		CmdOutputStreamReader cosr(cmd);
		return cosr;
	}

	static void testCmdOutputStreamReader() { //如果可以使用boost库，可以用另外的方法： http://www.cplusplus.com/forum/general/16532/
		//LC::CmdOutputStreamReader zfr("hello.txt.zip");
		LC::CmdOutputStreamReader zfr = LC::CmdOutputStreamReader::readZipFile("hello.txt.zip");
		std::istream& is = zfr.get_istream_ref();
		if (!zfr) {
			std::cout << "file not opened" << std::endl;
		}

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
/**
 //execute shell command
 //执行一个shell命令，输出结果逐行存储在resvec中，并返回行数
 static int32_t myexec(const char *cmd, std::vector<std::string> &resvec) { //https://www.cnblogs.com/caosiyang/archive/2012/06/25/2560976.html
 resvec.clear();
 std::FILE *pp = ::popen(cmd, "r"); //建立管道
 if (!pp) {
 return -1;
 }

 boost::fdistream fs(fileno(pp));
 std::string line;
 while (std::getline(fs, line)) {
 std::cout << line << std::endl;
 }

 char tmp[3]; //设置一个合适的长度，以存储每一行输出
 //    string s;
 //
 //    while(std::getline(pp,s)){
 //    	std::cout<<s<<std::endl;
 //    }
 //
 while (::fgets(tmp, sizeof(tmp), pp) != NULL) {
 if (tmp[strlen(tmp) - 1] == '\n') {
 tmp[strlen(tmp) - 1] = '\0'; //去除换行符
 }
 resvec.push_back(tmp);
 }
 ::pclose(pp); //关闭管道
 return resvec.size();
 }
 */
