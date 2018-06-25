/*
 * ContainerUtil.hpp
 *
 *  Created on: Nov 29, 2017
 *      Author: colinliang
 */

#ifndef LC_UTILITY_CONTAINERUTIL_HPP_
#define LC_UTILITY_CONTAINERUTIL_HPP_
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include <sstream>

namespace LC {

class ContainerUtil {
public:
	template<typename K, typename V>
	static const V& get(const std::map<K, V>& m, const K& key, const V& default_value) { //类似python dict的get函数, 参考自 https://stackoverflow.com/questions/2333728/stdmap-default-value
		typename std::map<K, V>::const_iterator it = m.find(key);
		if (it == m.end()) {
			return default_value;
		} else {
			return it->second;
		}
	}

	template<typename K, typename V>
	static const V& get(const std::unordered_map<K, V>& m, const K& key, const V& default_value) { //类似python dict的get函数, 参考自 https://stackoverflow.com/questions/2333728/stdmap-default-value
		typename std::unordered_map<K, V>::const_iterator it = m.find(key);
		if (it == m.end()) {
			return default_value;
		} else {
			return it->second;
		}
	}

	template<typename V>
	static void filter_in_place(std::vector<V>& v,std::function<bool (const V&)> is_valid_func) {
		unsigned int to_insert_idx=0;
		for(unsigned int i=0;i<v.size();++i) {
			if(is_valid_func(v[i])) {
				if(to_insert_idx!=i) {
					v[to_insert_idx]=std::move(v[i]);
				}
				to_insert_idx++;
			}
		}
		v.resize(to_insert_idx);
	}

	template<typename K, typename V>
	static std::string toString(const std::map<K, V>& m, int max_num_to_show = 10) { //类似python dict的get函数, 参考自 https://stackoverflow.com/questions/2333728/stdmap-default-value
		std::stringstream ss;
		ss << "{";
		int count = 0;
		for (auto it = m.begin(); it != m.end(); ++it) {
			ss << "(" << it->first << ", " << it->second << "), ";
			count++;
			if (count >= max_num_to_show) {
				ss << m.size() - max_num_to_show << " items not show ...";
				break;
			}
		}
		ss << "}";
		return ss.str();
	}

	template<typename K, typename V>
	static void add_to(std::map<K, V>& m, const K& key, const V& increment_val) { //类似python dict的get函数, 参考自 https://stackoverflow.com/questions/2333728/stdmap-default-value
		typename std::map<K, V>::iterator it = m.find(key);
		if (it == m.end()) {
			m[key] = increment_val;
		} else {
			it->second += increment_val;
		}
	}

	template<typename K, typename V>
	static void add_to(std::unordered_map<K, V>& m, const K& key, const V& increment_val) { //类似python dict的get函数, 参考自 https://stackoverflow.com/questions/2333728/stdmap-default-value
		typename std::unordered_map<K, V>::iterator it = m.find(key);
		if (it == m.end()) {
			m[key] = increment_val;
		} else {
			it->second += increment_val;
		}
	}

	/** 更通用的版本：
	 template <template<class,class,class...> class C, typename K, typename V, typename... Args>
	 V GetWithDef(const C<K,V,Args...>& m, K const& key, const V & defval)
	 {
	 typename C<K,V,Args...>::const_iterator it = m.find( key );
	 if (it == m.end())
	 return defval;
	 return it->second;
	 }
	 */
};
}

#endif /* LC_UTILITY_CONTAINERUTIL_HPP_ */
