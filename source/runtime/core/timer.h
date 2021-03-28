#pragma once
#include <stdint.h>
#include <chrono>

namespace flower
{
	namespace global_timer
	{
		// ���ü�ʱ��
		void reset();

		// ��ȡ���ϴε��� reset() ���ȥ��ʱ�䣨�룩
		double get_timer_second();

		// ��ȡ���ϴε��� reset() ���ȥ��ʱ�䣨���룩
		double get_timer_milli_sec();

		// ��ȡ���ϴε��� reset() ���ȥ��ʱ�䣨΢�룩
		long long get_timer_micro_sec();
	}
}