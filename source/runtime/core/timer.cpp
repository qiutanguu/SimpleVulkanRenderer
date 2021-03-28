#include "timer.h"

namespace flower
{
	namespace global_timer
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> start;
	}
	
	void global_timer::reset()
	{
		start = std::chrono::high_resolution_clock::now();
	}

	double global_timer::get_timer_second()
	{
		return get_timer_micro_sec() * 0.000001;
	}

	double global_timer::get_timer_milli_sec()
	{
		return get_timer_micro_sec() * 0.001;
	}

	long long global_timer::get_timer_micro_sec()
	{
		// 当前时钟减去reset后时钟的计数
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
	}
}