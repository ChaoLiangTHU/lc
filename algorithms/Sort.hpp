#pragma once

#include <vector>
#include <algorithm>
#include <utility>

namespace LC {

namespace Private {

template<class T> class SortingIndexAscending {
	// 参考：http://bytes.com/topic/c/answers/132045-sort-get-index
	// 	http://www.alecjacobson.com/weblog/?p=1527
	T* arr;
public:
	SortingIndexAscending(T* p) :
			arr(p) {
	}
	bool operator()(const int ind1, const int ind2) const {
		return arr[ind1] < arr[ind2];
	}
};

template<class T> class SortingIndexDescending {
	T* arr;
public:
	SortingIndexDescending(T* p) :
			arr(p) {
	}
	bool operator()(const int ind1, const int ind2) const {
		return arr[ind1] > arr[ind2];
	}
};

template<typename First, typename Second>
class cmpPair_by_secondElem_Less {  // 将pair的第二个元素作为键值进行对比，， 小于，，放到sort方法中将按升序排列
public:
	bool operator()(const std::pair<First, Second>& a, const std::pair<First, Second>& b) {
		return b.second < a.second;
	}
};

template<typename First, typename Second>
class cmpPair_by_secondElem_Greater { // 将pair的第二个元素作为键值进行对比，， 大于，，放到sort方法中将按降序排列
public:
	bool operator()(const std::pair<First, Second>& a, const std::pair<First, Second>& b) {
		return b.second < a.second;
	}
};

}
class Sort {
public:

	/**
	 * 对数组内的元素进行排序（升序），采用插入排序, 平均O(N^2)，最好O(N)，即在已排序队列中插入新加入元素到适当的位置
	 *@param arrays 数组
	 *@param length 数组长度
	 */
	template<class T> static void insertionSortAscending(T* arrays, const int length) {
		T tmp;
		for (int j = 1; j < length; j++) {
			tmp = arrays[j];
			int i = j - 1;
			while (i >= 0 && arrays[i] > tmp) {
				arrays[i + 1] = arrays[i];
				--i;
			}
			arrays[i + 1] = tmp;
		}
	}

	/**
	 * 对数组内的元素进行排序（升序），采用冒泡排序, O(N^2)，即
	 *@param arrays 数组
	 *@param length 数组长度
	 */
	template<class T> static void bubbleSortAscending(T* arrays, const int length) {
		for (int j = length - 1; j >= 0; --j) {
			const int ii = j - 1;
			for (int i = 0; i <= ii; i++) {
				if (arrays[i] > arrays[i + 1]) {
					//swap
					T tmp = arrays[i];
					arrays[i] = arrays[i + 1];
					arrays[i + 1] = tmp;
				}
			}
		}
	}

	/**
	 * 对数组内的元素进行排序（升序），采用选择排序, O(N^2)，即每次选出剩余元素中最小的放在当前插入点
	 *@param arrays 数组
	 *@param length 数组长度
	 */
	template<class T> static void selectionSortAscending(T* arrays, const int length) {
		for (int j = 0; j < length - 1; j++) {
			int minIdx = j;
			for (int i = j; i < length; i++) {
				if (arrays[i] < arrays[minIdx]) {
					minIdx = i;
				}
			}
			if (j != minIdx) {
				//swap
				T tmp = arrays[j];
				arrays[j] = arrays[minIdx];
				arrays[minIdx] = tmp;
			}
		}
	}

	/**
	 * 对数组内的元素进行排序（升序），采用stl的稳定排序
	 *@param arrays 数组
	 *@param length 数组长度
	 */
	template<class T> static void sortStableAscending(T* arrays, const int length) {
		std::stable_sort(arrays, arrays + length);
	}

	/**
	 * 对数组内的元素进行排序（降序），采用stl的稳定排序
	 *@param arrays 数组
	 *@param length 数组长度
	 */
	template<class T> static void sortStableDescending(T* arrays, const int length) {
		std::stable_sort(arrays, arrays + length, std::greater<T>());
	}

public:
	/**
	 * 对数组内的元素进行排序（升序），不改变原始数组的内容，采用stl的稳定排序
	 *@param arrays 数组
	 *@param length 数组长度
	 *@return 下标数组，其中第i个元素对应 原始数组中第i小的元素在原始数组中的下标; 即arrays[returnValue[0]]为arrays中的最小值
	 */
	template<class T> static std::vector<int> sortIndexStableAscending(T* arrays, const int length) {
		std::vector<int> indexes(length);
		for (int i = 0; i < length; i++)
			indexes[i] = i;
		std::stable_sort(indexes.data(), indexes.data() + length, Private::SortingIndexAscending<T>(arrays));
		return indexes;
	}

	/**
	 * 对数组内的元素进行排序（降序），不改变原始数组的内容，采用stl的稳定排序
	 *@param arrays 数组
	 *@param length 数组长度
	 *@return 下标数组，其中第i个元素对应 原始数组中第i小的元素在原始数组中的下标; 即arrays[returnValue[0]]为arrays中的最大值
	 */
	template<class T> static std::vector<int> sortIndexStableDescending(T* arrays, const int length) {
		std::vector<int> indexes(length);
		for (int i = 0; i < length; i++)
			indexes[i] = i;
		std::stable_sort(indexes.data(), indexes.data() + length, Private::SortingIndexDescending<T>(arrays));
		return indexes;
	}

	/**
	 *	对数pair数组， 按升序排列，键值为第二个元素
	 */
	template<typename First, typename Second>
	static void sortPair_by_sencondElem_ascending(std::pair<First, Second>* begin, const int length) {
		std::sort(begin,begin+length,Private::cmpPair_by_secondElem_Less<First, Second>());
	}

	/**
	 *	对数pair数组， 按降序排列，键值为第二个元素
	 */
	template<typename First, typename Second>
	static void sortPair_by_sencondElem_Descending(std::pair<First, Second>* begin, const int length) {
		std::sort(begin,begin+length,Private::cmpPair_by_secondElem_Greater<First, Second>());
	}



};

}
