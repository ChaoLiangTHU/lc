/*
 * flannUtil.hpp
 *
 *  Created on: May 8, 2017
 *      Author: colinliang
 */

#ifndef LC_MACHINELEARNING_NEARESTNEIGHBOR_FLANNUTIL_HPP_
#define LC_MACHINELEARNING_NEARESTNEIGHBOR_FLANNUTIL_HPP_

#ifndef HAS_INCLUDE_FLANN
#define HAS_INCLUDE_FLANN
#include <flann/flann.hpp>
#endif

#include <stdio.h>
#include <iostream>
#include <vector>
#include <utility>
using std::vector;

namespace flann {
/**
 * 修改自L2距离   /flann/src/cpp/flann/algorithms/dist.h
 * 计算的是cosine similarity 的相反数
 */
template<class T>
struct QuasiCosineDistance {
	typedef bool is_kdtree_distance; //看文档，，只有定义了这个才能用使用了kdtree的近似算法

	typedef T ElementType;
	typedef typename Accumulator<T>::Type ResultType;

	/**
	 *  Compute the opposite number of cosine similarity between two vectors.
	 *
	 *	This is highly optimised, with loop unrolling, as it is one
	 *	of the most expensive inner loops.
	 *
	 *	The computation of norm at the end is omitted for
	 *	efficiency.
	 */
	template<typename Iterator1, typename Iterator2>
	ResultType operator()(Iterator1 a, Iterator2 b, size_t size, ResultType worst_dist = -1) const {
		ResultType result = ResultType();
		ResultType diff0, diff1, diff2, diff3;
		Iterator1 last = a + size;
		Iterator1 lastgroup = last - 3;
		//std::cout<<"a: "<<a<<std::endl;
		/* Process 4 items with each loop for efficiency. */
		while (a < lastgroup) {
			result += (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
			a += 4;
			b += 4;

			if ((worst_dist > 0) && (result > worst_dist)) {
				return result;
			}
		}
		/* Process last 0-3 pixels.  Not needed for standard vector lengths. */
		while (a < last) {
			result += *a++ * *b++;
		}
		return -result;
	}

	/**
	 *	Partial euclidean distance, using just one dimension. This is used by the
	 *	kd-tree when computing partial distances while traversing the tree.
	 *
	 *	Squared root is omitted for efficiency.
	 */
	template<typename U, typename V>
	inline ResultType accum_dist(const U& a, const V& b, int) const {
		return -a * b;
	}
};

template<typename DistanceClass = QuasiCosineDistance<float>>
class KDTreeNN {
public:
	int ndata;
	int dim;
	Matrix<float> dataset;
	Index<DistanceClass> index; //无法声明实例，因为没有默认构造函数。。。。

	KDTreeNN() :
			ndata(-1), dim(-1), index(flann::KDTreeIndexParams(1)) {

	}

	void buildTree(float* pdata, int ndata, int dim, int numTrees = 8) {
		this->ndata = ndata;
		this->dim = dim;
		dataset = Matrix<float>(pdata, ndata, dim);
		index = Index < DistanceClass > (dataset, flann::KDTreeIndexParams(numTrees));
		index.buildIndex();
	}

	std::vector<std::pair<int, float> > knnSearch(float* pquery, const int N, int checks = 128, int cores = 0) { ///< N为返回的neighborhood数量
		std::vector < std::pair<int, float> > idx_sim(N);
		Matrix<float> query(pquery, 1, dim);

		int* pidx = new int[N];
		Matrix<int> indices(pidx, 1, N);

		float* pdist = new float[N];
		Matrix<float> dists(pdist, 1, N);
		flann::SearchParams sp(checks);
		sp.cores = cores;
		index.knnSearch(query, indices, dists, N, sp);
		for (int i = 0; i < N; ++i) {
			idx_sim[i] = std::pair<int, float>(pidx[i], -pdist[i]);
		}
		delete[] pidx;
		delete[] pdist;
//		delete[] dists.ptr();
		return idx_sim;
	}

};

template<typename DistanceClass = QuasiCosineDistance<float>>
class KMeansNN {
public:
	int ndata;
	int dim;
	Matrix<float> dataset;
	Index<DistanceClass> index; //无法声明实例，因为没有默认构造函数。。。。

	KMeansNN() :
			ndata(-1), dim(-1), index(flann::KMeansIndexParams()) {

	}

	KMeansNN& operator =(const KMeansNN& other) {
#ifdef LCDebug
//		std::cout<<"WARNING: fake operator =, it is a clear method actually, because FLANN KMeansIndex operator = coredump when copying from new Index()"<<std::endl;

		std::cout<<"WARNING: hand wrote operator =, because FLANN KMeansIndex operator = coredump when copying from new Index()"<<std::endl;
		std::cout.flush();
#endif
		ndata = other.ndata;
		dim = other.dim;
		dataset = other.dataset;
//		std::cout << "veclen: " << other.index.veclen() << std::endl;
		if (other.index.veclen() > 0)
			index = other.index;
		else
			index = Index < DistanceClass > (flann::KMeansIndexParams());
		return *this;
	}

	void buildTree(float* pdata, int ndata, int dim, int branching = 32, int iterations = 11, float cb_index = 0.2) {
		this->ndata = ndata;
		this->dim = dim;
		dataset = Matrix<float>(pdata, ndata, dim);
		index = Index < DistanceClass
				> (dataset, flann::KMeansIndexParams(branching, iterations, FLANN_CENTERS_RANDOM, cb_index));
		index.buildIndex();
	}

	std::vector<std::pair<int, float> > knnSearch(float* pquery, const int N, int checks = 128, int cores = 0) { ///< N为返回的neighborhood数量
		std::vector < std::pair<int, float> > idx_sim(N);
		Matrix<float> query(pquery, 1, dim);

		int* pidx = new int[N];
		Matrix<int> indices(pidx, 1, N);

		float* pdist = new float[N];
		Matrix<float> dists(pdist, 1, N);
		flann::SearchParams sp(checks);
		sp.cores = cores;
		index.knnSearch(query, indices, dists, N, sp);
		for (int i = 0; i < N; ++i) {
			idx_sim[i] = std::pair<int, float>(pidx[i], -pdist[i]);
		}
		delete[] pidx;
		delete[] pdist;
//		delete[] dists.ptr();
		return idx_sim;
	}

};

}

#endif /* LC_MACHINELEARNING_NEARESTNEIGHBOR_FLANNUTIL_HPP_ */
