#pragma once
#include <time.h>
#include <chrono>
namespace LC {
class Timer {
private:
	clock_t time;
public:
	Timer() {
		time = clock();
	}

	clock_t getElapsedTime() {
		return clock() - time;
	}

	clock_t getElapsedTimeAndRestart() {
		clock_t tmp = time;
		time = clock();
		return time - tmp;
	}

	double getElapseTime_in_second_and_restart() {
		double e = (getElapsedTimeAndRestart() / (double) getCLOCKS_PER_SEC());
		return e;
	}

	void restart() {
		time = clock();
	}

	int getCLOCKS_PER_SEC() {
		return CLOCKS_PER_SEC;
	}
};

class TimerAccurate {
private:
	std::chrono::system_clock::time_point start;
public:
	TimerAccurate() {
		start = std::chrono::system_clock::now();
	}

	inline double getElapsedTime() { //单位 s
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		return elapsed_seconds.count();
	}

	inline long long getElapsedTime_in_ms() {
		auto end = std::chrono::system_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	}

	inline double getElapsedTimeAndRestart() {
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		start = end;
		return elapsed_seconds.count();
	}

	inline double getElapseTime_in_second_and_restart() {
		return getElapsedTimeAndRestart();
	}

	inline void restart() {
		start = std::chrono::system_clock::now();
	}

	inline int getCLOCKS_PER_SEC() {
		return 1;
	}
};

}
