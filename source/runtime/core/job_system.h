#pragma once
#include <stdint.h>
#include <functional>

namespace flower
{
	namespace job_system
	{
		// 初始化。
		void initialize();

		/// <summary>
		/// 添加任务以异步执行。
		/// </summary>
		/// <param name="job">要添加的任务</param>
		void execute(const std::function<void()>& job);


		// 线程分发参数
		struct dispatch_args
		{
			uint32_t job_id;
			uint32_t group_id;
		};

		/// <summary>
		/// 将一个任务分发到多个子任务中并行执行。
		/// </summary>
		/// <param name="job_count"></param>
		/// <param name="group_size"></param>
		/// <param name="job">需要分发的任务</param>
		void dispatch(const uint32_t& job_count,const uint32_t& group_size,const std::function<void(dispatch_args)>& job);

		// 检测是否有任务正在工作。
		bool is_busy();

		// 等待所有任务执行完毕。
		void wait_for_all();

		void destroy();
	}
}

