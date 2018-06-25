#define EIGEN_DONT_PARALLELIZE
#include <Eigen/Dense>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "BinaryIO.hpp"

namespace LC {

template<typename T>
static std::string eigenMatrix2string(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> mat) {
	std::stringstream ss;
	for (unsigned int r = 0; r < mat.rows(); ++r) {
		for (unsigned int c = 0; c < mat.cols(); ++c) {
			ss << mat(r, c) << ", ";
		}
		ss << std::endl;
	}
	return ss.str();
}

template<typename T>
static std::string eigenRowVec2string(Eigen::Matrix<T, 1, Eigen::Dynamic> rowvec) {
	std::stringstream ss;
	for (unsigned int c = 0; c < rowvec.size(); ++c) {
		ss << rowvec(c) << ", ";
	}
//	ss<<std::endl;
	return ss.str();
}

template<typename T>
static std::string eigenColVec2string(Eigen::Matrix<T, Eigen::Dynamic, 1> colvec) {
	std::stringstream ss;
	for (unsigned int c = 0; c < colvec.size(); ++c) {
		ss << colvec(c) << "; ";
	}
	ss<<std::endl;
	return ss.str();
}



/**
 * 读取存有数据矩阵的文件.
 * 文件格式：行数，列数，按列优先存储的矩阵数据
 *@param filename 文件名
 *@return
 */
template<typename T>
static Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> loadEigenMatrix_v2(std::string filename) {
	BinaryFileIO bf(filename.c_str(), std::ios::in | std::ios::binary);
#define MaxSize 1024
	char buffer[MaxSize];
	bf.getline(buffer, MaxSize);  //  f32x24  表示 float型的 32行x24列的矩阵  f32表示行向量
	std::string typeStr = std::string("1");  //f
	typeStr[0] = buffer[0];
	std::string line(buffer + 1);  // 32x24

	if (typeStr.compare(DType<T>::toStr()) != 0) {
		std::cout << typeStr << std::endl;
		throw std::string("type not compatible: ").append(DType<T>::toStr()).append(":").append(typeStr);
	}

	std::vector<string> strs = Str::split(line, 'x');
	int rows = 1, cols = 1;
	if (strs.size() == 1) { //默认行向量
		cols = Str::str2int((char*) (strs[0].data()));
	} else if (strs.size() == 2) {
		rows = Str::str2int((char*) (strs[0].data()), (char**) 0);
		cols = Str::str2int((char*) (strs[1].data()), (char**) 0);
	} else {
		throw std::string("Unsupported matrix dim");
	}

	Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m(rows, cols);
	for (int r = 0; r < rows; r++)
		for (int c = 0; c < cols; c++) {
			{
				m(r, c) = bf.readBinaryNumber<T>();
			}
		}
	bf.close();
	return m;
}

template<typename T>
static void saveEigenMatrix_v2(std::string filename, Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m) {
	BinaryFileIO bf(filename.c_str(), std::ios::out | std::ios::binary);
	const int rows = m.rows();
	const int cols = m.cols();
	std::string str = DType<T>::toStr();
	bf << str;
	bf << m.rows() << 'x' << m.cols() << '\n';

	for (int r = 0; r < rows; r++)
		for (int c = 0; c < cols; c++) {
			{
				bf.writeBinaryNumber<T>(m(r, c));
			}
		}
	bf.close();
}

/**
 * 读取存有数据矩阵的文件.
 * 文件格式：行数，列数，按列优先存储的矩阵数据
 *@param filename 文件名
 *@return
 */
template<typename T>
static Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> loadEigenMatrix(std::string filename) {
	BinaryFileIO bf(filename.c_str(), std::ios::in | std::ios::binary);
#define MaxSize 1024
	char buffer[MaxSize];
	bf.getline(buffer, MaxSize);
	std::string line(buffer);
	std::vector<string> strs = Str::split(line, '(');
	strs = Str::split(strs[1], ')');
	strs = Str::split(strs[0], ',');

	if (Str::trim(strs[strs.size() - 1]).size() == 0)
		strs.pop_back();

	if (strs.size() > 2)
		throw std::string("Unsupported dim");
	std::vector<int> dims;
	long long d = 1;
	for (unsigned int i = 0; i < strs.size(); ++i) {
		int dd = Str::str2int((char*) (strs[i].data()), (char**) 0);
		dims.push_back(dd);
		d *= dd;
	}

	bf.getline(buffer, MaxSize);
	line = std::string(buffer);
	line = Str::trim(line);
	if (line.compare(DType<T>::toStr()) != 0)
		throw std::string("type not compatible: ").append(DType<T>::toStr()).append(":").append(line);

	int rows = 1;
	int cols = 1;
	if (dims.size() == 1) {
		cols = dims[0];
	} else {
		rows = dims[0];
		cols = dims[1];
	}

	Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m(rows, cols);
	for (int r = 0; r < rows; r++)
		for (int c = 0; c < cols; c++) {
			{
				m(r, c) = bf.readBinaryNumber<T>();
			}
		}
	bf.close();
	return m;
}

template<typename T>
static void saveEigenMatrix(std::string filename, Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m) {
	BinaryFileIO bf(filename.c_str(), std::ios::out | std::ios::binary);
	const int rows = m.rows();
	const int cols = m.cols();
	bf << "#shape=(";
	if (rows == 1)
		bf << cols << ",)\n";
	else
		bf << rows << ',' << cols << ")\n";

	std::string str = DType<T>::toStr();
	bf << str << "\n";

	for (int r = 0; r < rows; r++)
		for (int c = 0; c < cols; c++) {
			{
				bf.writeBinaryNumber<T>(m(r, c));
			}
		}
	bf.close();
}

}

