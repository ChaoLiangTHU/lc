/*
 * Discretize.hpp
 *
 *  Created on: Apr 1, 2017
 *      Author: colinliang
 */

#ifndef LC_MACHINELEARNING_PREPROCESS_DISCRETIZE_HPP_
#define LC_MACHINELEARNING_PREPROCESS_DISCRETIZE_HPP_

#include <vector>
#include <algorithm>
//#include <unordered_map>

#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include "../../utility/StringUtil.hpp"
#include "../../IO/CSVReader.hpp"

namespace LC {
namespace Discretize {

template<typename SrcType, typename DstType = int>
class Discretize {
public:
	virtual ~Discretize() = default;
	virtual DstType operator()(const SrcType& value)=0;
};

template<typename SrcType, typename DstType = int>
class Bucketize: public Discretize<SrcType, DstType> {
public:
	Bucketize() {

	}
	virtual ~Bucketize() = default;

	Bucketize(std::string paramStr) {
		boundaries_ = LC::Str::str2numVec<SrcType>(paramStr);
	}

	Bucketize(std::vector<SrcType> boundaries) :
			boundaries_(boundaries) {

	}
	virtual DstType operator()(const SrcType& value) {
		return static_cast<DstType>(std::upper_bound(boundaries_.begin(), boundaries_.end(), value)
				- boundaries_.begin());
	}
private:
	std::vector<SrcType> boundaries_;
};

template<typename DstType = int>
class IP2int: public Discretize<std::string, DstType> {
public:
	virtual ~IP2int() {

	}
	IP2int(std::string paramStr) {
		shift = LC::Str::str2num<int>(paramStr);
	}

	IP2int(int shift = 8) :
			shift(shift) {
	}
	virtual DstType operator()(const std::string& value) {
		DstType dst { };
		auto ints = LC::Str::str2numVec<int>(value);
		for (auto i : ints) {
			dst = dst << 8 | i;
		}
		dst >>= shift;
		return dst;
	}
private:
	int shift;
};

template<typename SrcType, typename DstType = int>
class Identity_with_default: public Discretize<SrcType, DstType> {
public:
	Identity_with_default() {

	}
	virtual ~Identity_with_default() {

	}
	Identity_with_default(std::string paramStr) {
		std::vector<long long> params = LC::Str::str2numVec<long long>(paramStr);
		if (params.size() < 4u)
			throw std::invalid_argument(std::string("param num less than 4: ") + paramStr);

		min_valid_value_ = (SrcType) params[0];
		max_valid_value_ = (SrcType) params[1];
		shift_ = (DstType) params[2];
		default_value_after_map_ = (DstType) params[3];
	}

	Identity_with_default(SrcType min_valid_value, SrcType max_valid_value, DstType shift,
			DstType default_value_after_map) :
			min_valid_value_(min_valid_value), max_valid_value_(max_valid_value), shift_(shift), default_value_after_map_(
					default_value_after_map) {

	}
	virtual DstType operator()(const SrcType& value) {
		return (value > max_valid_value_ || value < min_valid_value_) ?
				default_value_after_map_ : (DstType) (value) + shift_;
	}
private:
	SrcType min_valid_value_, max_valid_value_;
	DstType shift_;
	DstType default_value_after_map_;
};

template<typename SrcType, typename DstType = int, typename WorkType = SrcType>
class Linear: public Discretize<SrcType, DstType> {
public:
	Linear() {

	}
	virtual ~Linear() {

	}
	Linear(std::string paramStr) {
		std::vector<WorkType> params = LC::Str::str2numVec<WorkType>(paramStr);
		if (params.size() < 4u)
			throw std::invalid_argument(
					std::string(
							"param num less than 4: ") + paramStr+"; \tat LINE: "+LC::Str::num2str(__LINE__)+"; \tFUNC:"+__FUNCTION__ +"; \tFILE:"+__FILE__);

		min_valid_value_ = (WorkType) params[0];
		max_valid_value_ = (WorkType) params[1];
		a_ = (WorkType) params[2];
		b_ = (WorkType) params[3];
	}

	Linear(WorkType min_valid_value, WorkType max_valid_value, WorkType a, WorkType b) :
			min_valid_value_(min_valid_value), max_valid_value_(max_valid_value), a_(a), b_(b) {

	}
	virtual DstType operator()(const SrcType& value) {
		WorkType v = value;
		if (v < min_valid_value_)
			v = min_valid_value_;
		else if (v > max_valid_value_)
			v = max_valid_value_;
		return static_cast<DstType>(a_ * v + b_);
	}
private:
	WorkType min_valid_value_, max_valid_value_;
	WorkType a_, b_;
};

/**
 *	a_ *log( max(min(x,max_value),min_value)+ shift)+b_
 */
template<typename SrcType, typename DstType = int, typename WorkType = SrcType>
class LogLinear: public Discretize<SrcType, DstType> {
public:
	LogLinear() {

	}
	virtual ~LogLinear() {

	}
	LogLinear(std::string paramStr) {
		std::vector<WorkType> params = LC::Str::str2numVec<WorkType>(paramStr);
		if (params.size() < 5u)
			throw std::invalid_argument(
					std::string(
							"param num less than 5: ") + paramStr+"; \tat LINE: "+LC::Str::num2str(__LINE__)+"; \tFUNC:"+__FUNCTION__ +"; \tFILE:"+__FILE__);

		min_valid_value_ = (WorkType) params[0];
		max_valid_value_ = (WorkType) params[1];
		a_ = (WorkType) params[2];
		b_ = (WorkType) params[3];
		shift_in_log_ = params[4];
	}

	LogLinear(WorkType min_valid_value, WorkType max_valid_value, WorkType a, WorkType b, WorkType shift_in_log) :
			min_valid_value_(min_valid_value), max_valid_value_(max_valid_value), a_(a), b_(b), shift_in_log_(
					shift_in_log) {
	}
	virtual DstType operator()(const SrcType& value) {
		WorkType v = value;
		if (v < min_valid_value_)
			v = min_valid_value_;
		else if (v > max_valid_value_)
			v = max_valid_value_;
		return static_cast<DstType>(a_ * std::log(v + shift_in_log_) + b_);
	}
private:
	WorkType min_valid_value_, max_valid_value_;
	WorkType a_, b_;
	WorkType shift_in_log_;
};

template<typename SrcType, typename DstType = int>
class SignOf: public Discretize<SrcType, DstType> {
public:
	SignOf(const SrcType th) :
			th(th) {

	}
	virtual ~SignOf() {

	}
	SignOf(std::string paramStr) {
		std::vector<SrcType> params = LC::Str::str2numVec<SrcType>(paramStr);
		if (!params.empty())
			th = params[0];
	}

	virtual DstType operator()(const SrcType& value) {
		if (value > th)
			return static_cast<DstType>(1);
		else if (value < -th)
			return static_cast<DstType>(-1);
		return static_cast<DstType>(0);
	}
private:
	SrcType th;
};

template<typename SrcType, typename DstType = int>
class Map: public Discretize<SrcType, DstType> {
public:
	Map() {
	}
	virtual ~Map() {
	}
	Map(std::string paramStr, std::string map_file_folder, DstType maxIndexInclude = -1) {
//		cout << "maxIndexInclude: " << maxIndexInclude << endl;
//		cout << "size of map (init): " << key2value_.size() << endl;
		std::vector<std::string> params = LC::Str::split(paramStr, ';'); //config_file, default_value, key_index, value_index
		if (map_file_folder[-1] != '\\' || map_file_folder[-1] != '/')
			map_file_folder += '/';
		std::string filename = map_file_folder + params[0];
		default_value_ = LC::Str::str2num<DstType>(params[1]);
		const int key_index = LC::Str::str2num<int>(params[2]);
		const int value_index = LC::Str::str2num<int>(params[3]);
		std::vector<vector<std::string> > key_idx_count_name = LC::CSVReader::readCSV(filename, '\a', false);
		for (unsigned int i = 0; i < key_idx_count_name.size(); ++i) {
			const std::vector<std::string>& kicn = key_idx_count_name[i];

			DstType idx = LC::Str::str2num<DstType>(kicn[value_index]);
			if (idx > 0)
				if (maxIndexInclude < 0 || idx <= maxIndexInclude) {
					key2value_[LC::Str::str2num<SrcType>(kicn[key_index])] = idx;
//					cout << LC::Str::str2num<SrcType>(kicn[key_index]) << " : " << idx << endl;
				}
		}
//		cout << "size of map: " << key2value_.size() << endl;
	}

	Map(std::map<SrcType, DstType>& key2value) :
			key2value_(key2value) {

	}
	virtual DstType operator()(const SrcType& value) {
		typename std::map<SrcType, DstType>::iterator it = key2value_.find(value);
		if (it == key2value_.end()) {
			return default_value_;
		} else {
			return it->second;
		}
	}
	const std::map<SrcType, DstType>& getMap() const {
		return key2value_;
	}

	std::map<SrcType, DstType>& getMapRef() {
		return key2value_;
	}

	DstType& operator[](const SrcType key) {
		return key2value_[key];
	}
private:
	std::map<SrcType, DstType> key2value_;
	DstType default_value_;
};

/**
 *
 * 基于 0~9，A-Z， a-z，对数字进行离散化的类, 每个数字编码为这62个可见字符中的一个;
 * 暂时不支持defaultvalue， 小于最小值的值被设为idx=0； 大于最大阈值的值被设为idx=max_idx，而不是二者均被设为0
 */
class Base62 {
public:
	static inline char idx2char(int idx) {
		const char* v = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#ifdef LCDebug
		const int len = 62;
		if(idx<0) throw string("idx <0, the idx is ")+std::to_string(idx);
		if(idx>=len) throw string("idx >max_idx, the idx is")+std::to_string(idx);
#endif
		return v[idx];
	}

	static string idx2str(int idx) {
		const char* vv = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		const int len = 62;
		string r;
#ifdef LCDebug
		if(idx<0) throw string("idx <0, the idx is ")+std::to_string(idx);
#endif

		int v[16];
		int n = 0;
		while (idx >= len) {
			auto qr = std::div(idx, len);
			v[n++] = qr.rem;
			idx = qr.quot;
		}
		v[n++] = idx;

		r.resize(n * 2 - 1);
//		std::cout << r.size() << std::endl;
		for (int i = n - 1; i > 0; --i) {
			int p = n - 1 - i;
			r[2 * p] = '*';
			r[2 * p + 1] = vv[v[i]];
		}

		r[r.size() - 1] = vv[v[0]];

		return r;
	}

	static inline int str2idx(const string& inputs, unsigned int& pos) {
		const int len = 62;
		int r = 0;
		do {

			if (inputs[pos] == '*') {
#ifdef LCDebug
				if(pos+2u>inputs.size())throw string("pos >inputs.size(), the pos is ")+std::to_string(pos);
#endif
				r = r * len + ch2idx(inputs[pos + 1]);
				pos += 2;
			} else {
#ifdef LCDebug
				if(pos>inputs.size())throw string("pos >inputs.size(), the pos is ")+std::to_string(pos);
#endif
				r = r * len + ch2idx(inputs[pos]);
				pos += 1;
				return r;
			}
		} while (true);
		return 0;
	}

	static string indice2str(vector<int> indice) {
		std::stringstream ss;
		for (unsigned int i = 0; i < indice.size(); ++i) {
			ss << idx2str(indice[i]);
		}
		return ss.str();
	}

	static vector<int> str2indice(const string& inputs) {
		vector<int> r;
		unsigned int pos = 0;
		while (pos < inputs.size()) {
			r.push_back(str2idx(inputs, pos));
		}

		return r;
	}

	static inline int int2idx(int input, int min_val_include = 0, int max_val_include = 61) {

#ifdef LCDebug
		const int len = 62;
		if(max_val_include-min_val_include>=len)throw std::string("invalid arg, more than 62 possible values");
#endif
//		const int idx = input;
		if (input <= min_val_include) {
			return 0;
		} else if (input >= max_val_include) {
			return max_val_include - min_val_include;
		} else {

			return input - min_val_include;
		}
	}
	/**
	 * 将最小值映射到 '0', 之后的值对应的字符依次增加；  注意，没有乘以任何因子， max_val_include 只是为了限制最大值
	 * @param input
	 * @param min_val_include
	 * @param max_val_include
	 * @return
	 */
	static char encode_int(int input, int min_val_include = 0, int max_val_include = 61) {
		const char* v = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#ifdef LCDebug
		const int len = 62;
		if(max_val_include-min_val_include+1>len)throw std::string("invalid arg, more than 62 possible values");
#endif
		if (input <= min_val_include) {
			return v[0];
		} else if (input >= max_val_include) {
			return v[max_val_include - min_val_include];
		} else {

			return v[input - min_val_include];
		}
	}

	static string encode_int(std::vector<int> input, int min_val_include, int max_val_include) {
		std::string res;
		res.resize(input.size());
		for (unsigned int i = 0; i < input.size(); ++i) {
			res[i] = encode_int(input[i], min_val_include, max_val_include);
		}
		return res;
	}

	static inline int float2idx(float input, float min_val_include = 0.0f, float max_val_include = 1.0f,
			const int num_bins = 62) {
//
#ifdef LCDebug
		const int len = 62;
		if(num_bins>len)throw std::string("can not use more than 62 bins");
		if(num_bins<0) throw std::string("can not use <0 bins");
#endif
		const int max_idx = num_bins - 1;
		float y = max_idx * (input - min_val_include) / (max_val_include - min_val_include) + 0.4999999f;
		int idx = static_cast<int>(y);
		if (idx <= 0)
			return 0;
		if (idx >= max_idx)
			return max_idx;
		return idx;
	}

	static char encode_float(float input, float min_val_include = 0.0f, float max_val_include = 1.0f,
			const int num_bins = 62) {
		const char* v = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		return v[float2idx(input, min_val_include, max_val_include, num_bins)];
	}

	static string encode_float(std::vector<float> input, float min_val_include = 0.0f, float max_val_include = 1.0f) {
		std::string res;
		res.resize(input.size());
		for (unsigned int i = 0; i < input.size(); ++i) {
			res[i] = encode_float(input[i], min_val_include, max_val_include);
		}
		return res;
	}

	static int ch2idx(char ch) {
		if (ch >= '0' && ch <= '9')
			return ch - '0';
		else if (ch >= 'A' && ch <= 'Z')
			return ch - 'A' + 10;
		else if (ch >= 'a' && ch <= 'z')
			return ch - 'a' + 36;
		throw string("invalid input, char must in 0~9, A~Z, or a~z");
		return -1;
	}

	static inline int decode_int(char input, int min_val_include, int max_val_include) {
		int idx = ch2idx(input);
		int r = min_val_include + idx;
		if (r > max_val_include)
			return max_val_include;
		else
			return r;
	}

	static inline float decode_float(char input, float min_val_include = 0.0f, float max_val_include = 1.0f,
			const int num_bins = 62) {
//		const int len = 62;
		const int max_idx = num_bins - 1;
		int idx = ch2idx(input);
		float r = idx * (max_val_include - min_val_include) / max_idx + min_val_include;
		if (r > max_val_include)
			return max_val_include;
		else
			return r;
	}

	static std::vector<int> decode_int(string input, int min_val_include, int max_val_include) {
		std::vector<int> r;
		r.resize(input.size());
		for (unsigned int i = 0; i < input.size(); ++i) {
			r[i] = decode_int(input[i], min_val_include, max_val_include);
		}
		return r;
	}

	static std::vector<float> decode_float(string input, float min_val_include = 0.0f, float max_val_include = 1.0f) {
		std::vector<float> r;
		r.resize(input.size());
		for (unsigned int i = 0; i < input.size(); ++i) {
			r[i] = decode_float(input[i], min_val_include, max_val_include);
		}
		return r;
	}

	static void testDiscretizeBase62() {
		std::cout << "\n---test LC::Discretize::Base62::encode_int" << std::endl;
		int min_v_include = 2;
		int max_v_include = 6;
		std::vector<int> vf;
		for (int i = -1; i < 12; ++i) {
			vf.push_back(i);
			char ch = LC::Discretize::Base62::encode_int(i, min_v_include, max_v_include);
			int decoded = LC::Discretize::Base62::decode_int(ch, min_v_include, max_v_include);
			std::cout << i << " => " << ch << "   => " << decoded << std::endl;
		}
		std::cout << LC::Discretize::Base62::encode_int(vf, min_v_include, max_v_include) << std::endl;
		{
			std::cout << "\n---test LC::Discretize::Base62::encode_float" << std::endl;
			float min_v_include = 2;
			float max_v_include = 6;
			std::vector<float> vf;
			for (int i = -2; i < 20; ++i) {
				float in = i / 2.0f;
				vf.push_back(in);
				char ch = LC::Discretize::Base62::encode_float(in, min_v_include, max_v_include);
				float decoded = LC::Discretize::Base62::decode_float(ch, min_v_include, max_v_include);
				std::cout << in << " => " << ch << "   => " << decoded << std::endl;
			}
			std::cout << LC::Discretize::Base62::encode_float(vf, min_v_include, max_v_include) << std::endl;
		}
		exit(0);
	}

};

}/* end of namespace Discretize */
} // end of namespace LC

#endif /* LC_MACHINELEARNING_PREPROCESS_DISCRETIZE_HPP_ */
