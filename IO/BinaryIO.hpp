/*
 * BinaryIO.hpp
 *
 *  Created on: 2016年8月17日
 *      Author: colinliang
 */

#ifndef BINARYIO_HPP_
#define BINARYIO_HPP_

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

namespace LC {

template<typename T>
class DType {
public:
	static std::string toStr() {
		return "f";
	}
};

template<>
class DType<float> {
public:
	static std::string toStr() {
		return "f";
	}
};

template<>
class DType<int> {
public:
	static std::string toStr() {
		return "i";
	}
};

template<>
class DType<long long> {
public:
	static std::string toStr() {
		return "l";
	}
};

template<>
class DType<double> {
public:
	static std::string toStr() {
		return "d";
	}
};

//template<typename TT>
//std::string dtype2typestr() {
//	return std::string("f");
//}
//
//template<>
//std::string dtype2typestr<float>() {
//	return std::string("f");
//}
//
//template<>
//std::string dtype2typestr<int>() {
//	return std::string("i");
//}
//
//template<>
//std::string dtype2typestr<long long>() {
//	return std::string("l");
//}
//template<>
//std::string dtype2typestr<double>() {
//	return std::string("d");
//}

class BinaryFileIO: public std::fstream {
public:

	BinaryFileIO(const char* filename, std::ios_base::openmode mode = std::ios::in | std::ios::binary) :
			std::fstream(filename, mode) {
		if (!this->is_open()) {
			std::stringstream ss;
			ss << "" "" << filename << "" " can not be openned.";
			std::cerr<<ss.str()<<std::endl;
			throw ss.str();
		}
	}

	~BinaryFileIO() {
		this->close();
	}

	template<typename T>
	inline T readBinaryNumber() {
		T t;
		this->read((char*) &t, sizeof(T));
		return t;
	}

	template<typename T>
	inline std::vector<T> readBinaryNumbers(unsigned int N) {
		std::vector<T> nums(N);
		this->read((char*) nums.data(), sizeof(T) * N);
		return nums;
	}

	template<typename T>
	inline T writeBinaryNumber(T t) {
		this->write((char*) &t, sizeof(T));
		return t;
	}

	template<typename T>
	inline void writeBinaryNumbers(T* t, unsigned int N) {
		this->write((char*) t, sizeof(T) * N);
	}

	inline int readInt() {
		int x;
		this->read((char*) &x, sizeof(int));
		return x;
	}

	inline float readFloat() {
		float f;
		this->read((char*) &f, sizeof(float));
		return f;
	}

};

}

#endif /* BINARYIO_HPP_ */
