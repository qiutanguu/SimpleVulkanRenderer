#pragma once
#include <stdint.h>
#include <functional>

namespace flower
{
	/*
	  简单的 job system 实现
	  适合并行无返回值的短小多任务并行执行。
	  调用顺序：
	  1. initialize();
	  2. execute() 或 dispatch();
	  3. wait_for_all();
	  4. destroy();
	  例子：
		对于任务void Job();
		串行执行如下：
		  global_timer::reset();
		 	Job();
		 	Job();
		 	Job();
		 	Job();
		 	Job();
		  global_timer::get_timer_milli_sec();
	  改成Jobsystem调用，如下：
	    global_timer::reset();
		  job_system::execute([]{Job();});
		  job_system::execute([]{Job();});
		  job_system::execute([]{Job();});
		  job_system::execute([]{Job();});
		  job_system::execute([]{Job();});
		  job_system::wait_for_all();
	    global_timer::get_timer_milli_sec();
	  所有任务将在job_system::wait_for_all()后执行完毕。
	*/
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

