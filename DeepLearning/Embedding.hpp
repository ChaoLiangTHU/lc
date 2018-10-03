/*
 * Embedding.hpp
 *
 *  Created on: Oct 3, 2018
 *      Author: colinliang
 */

#ifndef LC_DEEPLEARNING_EMBEDDING_HPP_
#define LC_DEEPLEARNING_EMBEDDING_HPP_

#include <utility>
#include <functional>
#include "FeedForwardNet.hpp"
namespace LC {
template<typename KeyType = long long, typename WeightType = float>
class SparseEmbedding {
public:
	virtual RowVec operator()(const std::vector<std::pair<KeyType, WeightType>>& kvs, const Mat& m)=0;

};

template<typename KeyType = long long, typename WeightType = float>
class SparseEmbedding_mean_div_n {
public:
	 RowVec operator()(const std::vector<std::pair<KeyType, WeightType>>& kvs, const Mat& m) {
		RowVec rv = RowVec::Zero(m.cols());
		if (kvs.empty()) {
			rv = m.row(0);
			return rv;
		}
		for (unsigned int j = 0; j < kvs.size(); j++) {
			const auto& tw = kvs[j];
			rv += m.row(tw.first) * tw.second;
		}
		rv /= kvs.size();

		return rv;
	}

};

template<typename KeyType = long long, typename WeightType = float>
class SparseEmbedding_mean {
public:
	RowVec operator()(const std::vector<std::pair<KeyType, WeightType>>& kvs, const Mat& m) {
		RowVec rv = RowVec::Zero(m.cols());
		if (kvs.empty()) {
			rv = m.row(0);
			return rv;
		}
		WeightType w;
		for (unsigned int j = 0; j < kvs.size(); j++) {
			const auto& tw = kvs[j];
			rv += m.row(tw.first) * tw.second;
			w += std::abs(tw.second);
		}
		rv /= w;

		return rv;
	}

};

template<typename KeyType = long long, typename WeightType = float>
class SparseEmbedding_sum {
public:
	RowVec operator()(const std::vector<std::pair<KeyType, WeightType>>& kvs, const Mat& m) {
		RowVec rv = RowVec::Zero(m.cols());
		if (kvs.empty()) {
			rv = m.row(0);
			return rv;
		}
		for (unsigned int j = 0; j < kvs.size(); j++) {
			const auto& tw = kvs[j];
			rv += m.row(tw.first) * tw.second;
		}

		return rv;
	}

};

template<typename KeyType = long long, typename WeightType = float>
class SparseEmbeddingFactory {
public:
	typedef std::function<RowVec(const std::vector<std::pair<KeyType, WeightType>>&, const Mat&)> SparseEmbeddingFunc;
	static SparseEmbeddingFunc get(const std::string& combiner, const std::string& params = "") {
		auto f = LC::Str::toLowerCase(combiner);
		std::cout<<"geting embedding func, combiner = "<<combiner<<std::endl;
		if (f == "mean_div_n" || f.empty()) {
			return SparseEmbedding_mean_div_n<KeyType, WeightType>();
		} else if (f == "mean") {
			return SparseEmbedding_mean<KeyType, WeightType>();
		} else if (f == "sum") {
			return SparseEmbedding_mean<KeyType, WeightType>();
		}

		std::stringstream ss;
		ss<<"\nERROR: unsupported SparseEmbedding type '"<<combiner<<"', at LINE: "<<__LINE__<<"; \tFUNC:"<<__FUNCTION__ <<"; \tFILE:"<<__FILE__;

		throw std::invalid_argument(ss.str());
		return SparseEmbedding_mean_div_n<KeyType, WeightType>();
	}
};

} //namepace LC
#endif /* LC_DEEPLEARNING_EMBEDDING_HPP_ */
