/*
 * Random.hpp
 *
 *  Created on: Feb 20, 2017
 *      Author: colinliang
 */

#ifndef LC_UTILITY_RANDOM_HPP_
#define LC_UTILITY_RANDOM_HPP_

#include <random>
#include <chrono>
#include <ctime>
#include <set>
namespace LC {

/**
 * 不太严格的uniform random 封装，同时封装了int型和float型的random，且这二者公用一个随机数生成器
 * 使用每种random前，需要重新设置参数：set_min_max_all_incude_int；set_min_max_all_incude_float
 * 注意， 无论int 和float的随机数，都是用的闭区间  [minValue, maxValue]
 */
template<typename IntType = int, typename FloatType = float>
class RandomUniform {
//    std::random_device rd;
//    std::mt19937_64 gen(rd());
	std::mt19937_64 gen;
	std::uniform_int_distribution<IntType> distribution_i;
	std::uniform_real_distribution<FloatType> distribution_f;
public:
	RandomUniform(IntType seed = 0) {
		gen.seed((long long) seed);
	}

	void set_seed(IntType seed = 0) {
		gen.seed(seed);
	}

	void set_seed_to_clock(){
		gen.seed((IntType)(std::clock()));
	}


	inline void set_param_i(IntType minVal = 0, IntType maxVal = 10) {
		distribution_i = std::uniform_int_distribution<IntType>(minVal, maxVal);
	}

	inline void set_param_f(FloatType minVal = 0.0, FloatType maxVal = 1.0) {
		distribution_f = std::uniform_real_distribution<FloatType>(minVal,
				maxVal);
	}

	inline IntType randi() {
		return distribution_i(gen);
	}
	inline FloatType randf() {
		return distribution_f(gen);
	}


};
}

#endif /* LC_UTILITY_RANDOM_HPP_ */
