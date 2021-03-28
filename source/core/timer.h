#pragma once
#include <stdint.h>
#include <chrono>

namespace flower
{
	namespace global_timer
	{
		// 重置计时器
		void reset();

		// 获取自上次调用 reset() 后过去的时间（秒）
		double get_timer_second();

		// 获取自上次调用 reset() 后过去的时间（毫秒）
		double get_timer_milli_sec();

		// 获取自上次调用 reset() 后过去的时间（微秒）
		long long get_timer_micro_sec();
	}
}