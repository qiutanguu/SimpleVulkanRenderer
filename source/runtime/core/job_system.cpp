#include "job_system.h"
#include "ring_buffer.h"
#include <algorithm>  
#include <atomic>   
#include <thread> 
#include <condition_variable> 
#include <stdint.h>

/**************************************************
* 
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
	{
		global_timer::reset();
		Job();
		Job();
		Job();
		Job();
		Job();
		global_timer::get_timer_milli_sec();
	}
	改成Jobsystem调用，如下：
	{
		global_timer::reset();
		job_system::execute([]{Job();});
		job_system::execute([]{Job();});
		job_system::execute([]{Job();});
		job_system::execute([]{Job();});
		job_system::execute([]{Job();});
		job_system::wait_for_all();
		global_timer::get_timer_milli_sec();
	}
	所有任务将在job_system::wait_for_all()后执行完毕。

*****************************************************
*/

namespace flower
{
	namespace job_system
	{
		// 线程数。
		uint32_t num_threads = 0;  

	    // 线程池大小
        constexpr size_t job_pool_capacity = 256;

		// 任务池。
		ring_buffer<std::function<void()>,job_pool_capacity> job_pool;  
		
		// 线程唤醒。
		std::condition_variable wake_condition;   
		std::mutex wake_mutex;    
		
		// 标志位。
		uint64_t current_bit = 0; 
		std::atomic<uint64_t> finished_bit;

		// 弹出线程
		inline void poll()
		{
			// 唤醒一个线程
			wake_condition.notify_one();

			// 允许线程重新调度
			std::this_thread::yield(); 
		}
	}

	void job_system::initialize()
	{
		finished_bit.store(0);

		// 计算CPU线程数。
		auto num_cores = std::thread::hardware_concurrency();
		num_threads = std::max(1u,num_cores);

		// 开始时就创建全部的工作线程。
		for(uint32_t thread_id = 0; thread_id<num_threads; thread_id++)
		{
			std::thread worker([]
			{
				// 初始化时使用空任务填满任务池
				std::function<void()> job;
				while(true)
				{
					// 弹出第一个任务。
					if(job_pool.pop_front(job))
					{
						// 如果发现存在任务则执行它。
						job();

						// 更新标志位。
						finished_bit.fetch_add(1); 
					}
					else
					{
						// 不会工作直到线程被唤醒。
						std::unique_lock<std::mutex> lock(wake_mutex);
						wake_condition.wait(lock);
					}
				}
			});

			// 释放控制权
			worker.detach();
		}
	}

	void job_system::execute(const std::function<void()>& job)
	{
		// 更新标志位
		current_bit += 1;

		// 循环压入一个任务直到成功。
		while(!job_pool.push_back(job))
		{
			// 若队列已满则唤醒一个线程
			poll();
		}

		// 唤醒一个线程。
		wake_condition.notify_one(); 
	}

	bool job_system::is_busy()
	{
		return finished_bit.load() < current_bit;
	}

	void job_system::wait_for_all()
	{
		while(is_busy())
		{
			poll();
		}
	}

	void job_system::destroy()
	{
		wait_for_all();
	}

	void job_system::dispatch(const uint32_t& job_count,const uint32_t& group_size,const std::function<void(job_system::dispatch_args)>& job)
	{
		if( job_count==0 || group_size ==0)
		{
			return;
		}

		// 计算线程组数
		const uint32_t group_count = (job_count + group_size - 1) / group_size;

		// 更新标志位
		current_bit += group_count;

		for(uint32_t group_index = 0; group_index < group_count; ++group_index)
		{
			// 为每个线程组生成对应的任务
			auto job_group = [job_count,group_size,job,group_index]()
			{
				const uint32_t group_job_fffset = group_index * group_size;
				const uint32_t group_job_end = std::min(group_job_fffset + group_size,job_count);

				dispatch_args args;
				args.group_id = group_index;

				for(uint32_t i = group_job_fffset; i<group_job_end; ++i)
				{
					args.job_id = i;
					job(args);
				}
			};

			// 循环压入一个任务直到成功。
			while(!job_pool.push_back(job_group))
			{
				poll();
			}

			// 唤醒一个线程
			wake_condition.notify_one();
		}
	}
}