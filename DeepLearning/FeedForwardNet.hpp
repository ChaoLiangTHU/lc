#pragma once
#ifndef HAS_INCLUDE_EIGEN
#define HAS_INCLUDE_EIGEN
#define EIGEN_DONT_PARALLELIZE
#include <Eigen/Core>
#endif

#include <vector>

#include <iostream>
#include <utility>
#include <map>

#include "../IO/BinaryIO.hpp"
#include "../utility/StringUtil.hpp"

using std::vector;
using std::string;

using std::cout;
using std::endl;

namespace LC {

typedef Eigen::MatrixXf Mat;
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor | Eigen::AutoAlign> MatRowMajor;
typedef Mat::Scalar Scalar;
typedef Eigen::Matrix<Scalar, 1, Eigen::Dynamic> RowVec;
typedef Eigen::Matrix<Scalar, Eigen::Dynamic, 1> ColVec;

class INetLayer {  // newwork layer only for forward pass
public:

	virtual ~INetLayer() {
	}
	;
	virtual Mat& forward(Mat& input)=0;
};

class INetLayer_Affine: public INetLayer {  //Affine和 AffineBatchNorm的逻辑是一样的，都使用Affine层即可
public:
	Mat W;
	RowVec b;
	INetLayer_Affine(const Mat& W, const RowVec& b) :
			W(W), b(b) {
	}

	virtual Mat& forward(Mat& input) {
#ifdef LCDebug2
		std::cout<<"----Affine:"<<std::endl;
#ifdef LCDebug3
		std::cout<<"W: "<<W<<std::endl;
		std::cout<<"b: "<<b<<std::endl;
#endif
		std::cout<<"input: "<<input<<std::endl;
#endif
		input = (input * W).rowwise() + b;
#ifdef LCDebug2
		std::cout<<"output: "<<input<<std::endl;
		std::cout<<"~~\n"<<std::endl;
#endif

		return input;
	}
};

class INetLayer_BatchNorm: public INetLayer {
public:
	RowVec W;
	RowVec b;
	INetLayer_BatchNorm(const RowVec& W, const RowVec& b) :
			W(W), b(b) {  // W, b 为已经归一化后的值，，其实与一般的AffineLayer相似，，但由于W是行向量，，只好单独写一层
	}

	virtual Mat& forward(Mat& input) {
#ifdef LCDebug2
		std::cout<<"----BN:"<<std::endl;
#ifdef LCDebug3
		std::cout<<"W: "<<W<<std::endl;
		std::cout<<"b: "<<b<<std::endl;
#endif
		std::cout<<"input: "<<input<<std::endl;
#endif
		input = input.array().rowwise() * W.array();
		input.rowwise() += b;
#ifdef LCDebug2
		std::cout<<"output: "<<input<<std::endl;
		std::cout<<"~~\n"<<std::endl;
#endif
		return input;
	}
};

class INetLayer_ReLU: public INetLayer {
public:
	INetLayer_ReLU() {
	}

	static Mat& relu(Mat& input) {
		const unsigned int sizeInput = input.rows() * input.cols();
		Scalar* data = input.data();
#pragma omp parallel for
		for (unsigned int i = 0; i < sizeInput; ++i) {
			if (data[i] < 0)
				data[i] = 0;
		}

		return input;
	}

	virtual Mat& forward(Mat& input) {
#ifdef LCDebug2
		std::cout<<"----ReLU:"<<std::endl;
		std::cout<<"input: "<<input<<std::endl;
#endif
		const unsigned int sizeInput = input.rows() * input.cols();
		Scalar* data = input.data();
#pragma omp parallel for
		for (unsigned int i = 0; i < sizeInput; ++i) {
			if (data[i] < 0)
				data[i] = 0;
		}
#ifdef LCDebug2
		std::cout<<"output: "<<input<<std::endl;
		std::cout<<"~~\n"<<std::endl;
#endif
		return input;
	}

};

class INetLayer_Residual_AffineReLU: public INetLayer {
public:
	Mat W;
	RowVec b;
	INetLayer_Residual_AffineReLU(const Mat& W, const RowVec& b) :
			W(W), b(b) {
	}
	virtual Mat& forward(Mat& input) {
		Mat tmp = (input * W).rowwise() + b;
		INetLayer_ReLU::relu(tmp);
		input += tmp;

		return input;
	}

};

/////////////////////////////////////////////////////////////
class FeedForwardNet {
public:
	vector<INetLayer*> layers;

	static string extractVarName(const string& varStr) { //"W=f256x128"    -->  "W"
		return LC::Str::split(varStr, '=')[0];
	}

	static std::pair<unsigned int, unsigned int> extractRowsAndCols(const string& varStr) { //"W=f256x128"    -->  (256,128)

		string WStr = LC::Str::split(varStr, '=')[1]; //W=f2x2-->  f2x2
		if (WStr[0] != 'f')
			throw string("feed forward net only support float matrix now");
		WStr = WStr.substr(1);
		int rows = 1, cols = 1;
		std::vector<string> strs = LC::Str::split(WStr, 'x');
		if (strs.size() == 1) { //默认行向量
			cols = LC::Str::str2int((char*) (strs[0].data()));
		} else if (strs.size() == 2) {
			rows = LC::Str::str2int((char*) (strs[0].data()));
			cols = LC::Str::str2int((char*) (strs[1].data()));
		} else {
			throw std::string("Unsupported matrix dim");
		}

		return std::pair<unsigned int, unsigned int>(rows, cols);
	}

	static Mat loadOneMat_from_binaryFile(LC::BinaryFileIO& bf, std::pair<unsigned int, unsigned int> rc) {
		unsigned int rows = rc.first;
		unsigned int cols = rc.second;
		Mat m(rows, cols);
		for (unsigned int r = 0; r < rows; r++) {
			for (unsigned int c = 0; c < cols; c++) {
				m(r, c) = bf.readBinaryNumber<float>();
//				std::cout << m(r, c) <<",";
			}
//			throw "asdf";
//			std::cout<<std::endl;
		}
		return m;
	}

	static std::map<string, Mat> loadLookUps(const std::string& filename, bool printVerboseInfo = false) {
		LC::BinaryFileIO bf(filename.c_str());
		std::string info;
		std::getline(bf, info);
		std::vector<string> vars = LC::Str::split(info, '&');
		cout << "\n-------- Loading embedding lookups: " << filename << endl;
		cout << "vars:" << info << endl;
		std::map<string, Mat> map_varName2Mat;
		for (unsigned int i = 0; i < vars.size(); ++i) {
			string varStr = vars[i];
			if (varStr.size() == 0)
				continue;
			cout << "\tloading " << varStr << endl;
			if (varStr.size() < 3)
				continue;  // maybe the last one is empty
			string matName = extractVarName(varStr);
			Mat m = loadOneMat_from_binaryFile(bf, extractRowsAndCols(varStr));
			map_varName2Mat[matName] = m;
			if (printVerboseInfo) {
				cout << matName << endl << m << endl;
				cout << "~~~~~~~~~~~~~" << endl;
			}
		}
		cout << endl;
		return map_varName2Mat;
	}

	std::string loadNetStructure(const std::string& filename, bool printVerboseInfo = false) {
		std::cout << "\n-------- Loading feed forward net: " << filename << std::endl;
		LC::BinaryFileIO bf(filename.c_str());
		std::string info;
		std::getline(bf, info);

		std::vector<string> layerStrs = LC::Str::split(info, '|');
		for (unsigned int idx = 0; idx < layerStrs.size(); ++idx) {  // layers
			std::string layerStr = layerStrs[idx];
			if (layerStr.size() < 3)
				continue;
			cout << "layer " << idx << ", \t";
			std::vector<string> layerParams = LC::Str::split(layerStr, '&');
			string layerName = layerParams[0];
			cout << "" << layerName << endl;
			for (unsigned int p = 1; p < layerParams.size(); ++p) {  // layer params,  for print
				cout << "\t" << layerParams[p] << endl;
//				vector<string> param = StringUtility<int>::splitString(layerParams[p], '=');
//				for (unsigned int pp = 0; pp < param.size(); ++pp) {
//					cout << "\t\t" << param[pp] << endl;
//				}

			}

			if (layerName.compare("Affine") == 0 || layerName.compare("Affine_BatchNorm") == 0) {  //
				if (layerParams.size() - 1 != 2)
					throw string("Affine or Affine_BatchNorm Layer should have two parameters");
				Mat W = loadOneMat_from_binaryFile(bf, extractRowsAndCols(layerParams[1]));
				Mat bMat = loadOneMat_from_binaryFile(bf, extractRowsAndCols(layerParams[2]));
				RowVec b = bMat;
				this->addAffine(W, b);

				if (printVerboseInfo) {
					cout << "W:" << endl << W << endl;
					cout << "b:" << endl << b << endl;
					cout << "~~~~~~~~~~~~~" << endl;
//					int a;
//					std::cin >> a;
				}

			} else if (layerName.compare("ReLU") == 0) {
				this->addReLU();
			} else if (layerName.compare("BatchNorm") == 0) {
				if (layerParams.size() - 1 != 2)
					throw string("BatchNorm Layer should have two parameters");
				Mat WMat = loadOneMat_from_binaryFile(bf, extractRowsAndCols(layerParams[1]));
				Mat bMat = loadOneMat_from_binaryFile(bf, extractRowsAndCols(layerParams[2]));
				RowVec W = WMat;
				RowVec b = bMat;
				this->addBatchNorm(W, b);

				if (printVerboseInfo) {
					cout << "W:" << endl << W << endl;
					cout << "b:" << endl << b << endl;
					cout << "~~~~~~~~~~~~~" << endl;
//					int a;
//					std::cin >> a;

				}

			} else if (layerName.compare("Residual_AffineReLU") == 0) {
				if (layerParams.size() - 1 != 2)
					throw string("Residual_AffineReLU Layer should have two parameters");
				Mat WMat = loadOneMat_from_binaryFile(bf, extractRowsAndCols(layerParams[1]));
				Mat bMat = loadOneMat_from_binaryFile(bf, extractRowsAndCols(layerParams[2]));
				RowVec b = bMat;
				this->addResidual_AffineReLU(WMat, b);

				if (printVerboseInfo) {
					cout << "W:" << endl << WMat << endl;
					cout << "b:" << endl << b << endl;
					cout << "~~~~~~~~~~~~~" << endl;
//					int a;
//					std::cin >> a;

				}
			} else {
				string s = "unsupported layer type: ";
				std::cout << s << std::endl;
				s += layerName;
				std::cerr << s << std::endl;
				throw s;
			}

		}
		std::cout << std::endl;
		return info;
	}

	~ FeedForwardNet() {
		for (unsigned int i = 0; i < layers.size(); ++i) {
			delete layers[i];
		}
	}

	void addAffine(const Mat& W, const Mat& b) {
		layers.push_back(new INetLayer_Affine(W, b));
	}

	void addBatchNorm(const RowVec& W, const Mat& b) {
		layers.push_back(new INetLayer_BatchNorm(W, b));
	}

	void addResidual_AffineReLU(const Mat& W, const RowVec& b) {
		layers.push_back(new INetLayer_Residual_AffineReLU(W, b));
	}

	void addReLU() {
		layers.push_back(new INetLayer_ReLU());
	}

	Mat forward(const Mat& input) {  ///< 会首先复制一份input， 最后的输出不会覆盖input
#ifdef LCDebug2
			std::cout<<"\n------Begin FFN forward:"<<std::endl;
#endif
		Mat tmp = input;
		for (unsigned int i = 0; i < layers.size(); ++i) {
			layers[i]->forward(tmp);
		}
#ifdef LCDebug2
		std::cout<<"------End FFN forward\n"<<std::endl;
#endif
		return tmp;
	}

	Mat& forward_mutableInput(Mat& input) { ///<与forward函数功能相同，但 input会变为output，较为节省内存
#ifdef LCDebug2
			std::cout<<"\n------Begin FFN forward:"<<std::endl;
#endif
		for (unsigned int i = 0; i < layers.size(); ++i) {
			layers[i]->forward(input);
		}
#ifdef LCDebug2
		std::cout<<"------End FFN forward\n"<<std::endl;
#endif
		return input;
	}
};

/////////////////////////////////////////////////////////////

class MatInputLoader {  //自动对input进行串联，，需要首先设置
	int idx;
	Mat m;
public:
	MatInputLoader(int rows = 0, int cols = 0) :
			idx(0), m(rows, cols) {
	}

	void clear() {
		idx = 0;
	}

	void setInputSize(int rows, int cols) {
		m.resize(rows, cols);
		idx = 0;
	}

	Mat& getInput() {
		idx = 0;
		return m;
	}

	void addSubInput(const Mat& in) {
		m.middleCols(idx, in.cols()) = in;
		idx += in.cols();
	}

};

class RowVecInputLoader {  //自动对input进行串联，，需要首先设置
	int idx;
	RowVec m;
public:
	RowVecInputLoader(int cols = 0) :
			idx(0), m(cols) {
	}

	void clear() {
		idx = 0;
	}

	int size() {
		return idx;
	}

	void setInputSize(int cols) {
		m.resize(cols);
		idx = 0;
	}

	RowVec& getInput() {
		return m;
	}

	RowVec getEffectiveInput() {
		return m.segment(0, idx);
	}

	void addSubInput(const RowVec& in) {
		m.segment(idx, in.cols()) = in;
		idx += in.cols();
	}

	void addSubInput(float in) {
		m(idx) = in;
		idx += 1;
	}
};

}
