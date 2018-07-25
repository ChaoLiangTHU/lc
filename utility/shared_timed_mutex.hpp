/*
 * ReadWriteLock.hpp
 *
 *  Created on: Jun 14, 2018
 *      Author: colinliang
 */

#ifndef LC_UTILITY_SHARED_TIMED_MUTEX_HPP_
#define LC_UTILITY_SHARED_TIMED_MUTEX_HPP_

#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <sstream>

namespace LC {

/**
 * 参考C++14 API实现，没有实现不带time的lock，因为不想加额外的std::condition_variable_any 开销。
 * 参考自：https://blog.csdn.net/tillmanSkate/article/details/77688744
 */
class shared_timed_mutex {
	std::timed_mutex m;
//	std::condition_variable_any cv;

	int readcount = 0;
	bool is_writing = false;
public:
	std::string toString() {
		std::ostringstream ss;
		ss << "LC::shared_timed_mutex: readcount = " << readcount << ", \tis_writing = " << is_writing;
		return ss.str();
	}
	template<class _Rep, class _Period>
	inline bool try_lock_shared_for(const std::chrono::duration<_Rep, _Period>& __rtime) {
		if (m.try_lock_for(__rtime)) {
			if (is_writing) {
				m.unlock();
				return false;
			} else {
				if (readcount >= 100000)
					throw std::invalid_argument("two many read threads!!!");

				readcount++;
				m.unlock();
				return true;
			}
		}
		return false;
	}

	inline void unlock_shared() {
		std::unique_lock<std::timed_mutex> lock(m);
		readcount--;
	}

	//write lock
//	void lock() {
//		std::unique_lock<std::timed_mutex> lock(m);
//		while (is_writing || readcount > 0) {
//			cv.wait(lock);
//		}
//		is_writing = true;
//	}

	template<class _Rep, class _Period>
	inline bool try_lock_for(const std::chrono::duration<_Rep, _Period>& __rtime) {
		if (m.try_lock_for(__rtime)) {
			if (is_writing || readcount > 0) {
				m.unlock();
				return false;
			} else {
				is_writing = true;
				m.unlock();
				return true;
			}
		}
		return false;
	}

	inline void unlock() {
		m.lock();
		is_writing = false;
		m.unlock();
//		cv.notify_all();
	}
};

class shared_timed_mutex_shared_guard {
public:
	shared_timed_mutex* pmut;
	shared_timed_mutex_shared_guard(shared_timed_mutex* pmut) :
			pmut(pmut) {
	}
	~shared_timed_mutex_shared_guard() {
		pmut->unlock_shared();
	}
};

}

#endif /* LC_UTILITY_SHARED_TIMED_MUTEX_HPP_ */
