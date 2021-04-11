#include "job_system.h"
#include "ring_buffer.h"
#include <algorithm>  
#include <atomic>   
#include <thread> 
#include <condition_variable> 
#include <stdint.h>

/**************************************************
	�򵥵� job system ʵ��
	�ʺϲ����޷���ֵ�Ķ�С��������ִ�С�
	����˳��
		1. initialize();
		2. execute() �� dispatch();
		3. wait_for_all();
		4. destroy();
	���ӣ�
	��������void Job();
	����ִ�����£�
	{
		global_timer::reset();
		Job();
		Job();
		Job();
		Job();
		Job();
		global_timer::get_timer_milli_sec();
	}
	�ĳ�Jobsystem���ã����£�
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
	����������job_system::wait_for_all()��ִ����ϡ�
*****************************************************/

namespace flower
{
	namespace job_system
	{
		// �߳�����
		uint32_t num_threads = 0;  

	    // �̳߳ش�С
        constexpr size_t job_pool_capacity = 256;

		// ����ء�
		ring_buffer<std::function<void()>,job_pool_capacity> job_pool;  
		
		// �̻߳��ѡ�
		std::condition_variable wake_condition;   
		std::mutex wake_mutex;    
		
		// ��־λ��
		uint64_t current_bit = 0; 
		std::atomic<uint64_t> finished_bit;

		// �����߳�
		inline void poll()
		{
			// ����һ���߳�
			wake_condition.notify_one();

			// �����߳����µ���
			std::this_thread::yield(); 
		}
	}

	void job_system::initialize()
	{
		finished_bit.store(0);

		// ����CPU�߳�����
		auto num_cores = std::thread::hardware_concurrency();
		num_threads = std::max(1u,num_cores);

		// ��ʼʱ�ʹ���ȫ���Ĺ����̡߳�
		for(uint32_t thread_id = 0; thread_id<num_threads; thread_id++)
		{
			std::thread worker([]
			{
				// ��ʼ��ʱʹ�ÿ��������������
				std::function<void()> job;
				while(true)
				{
					// ������һ������
					if(job_pool.pop_front(job))
					{
						// ������ִ���������ִ������
						job();

						// ���±�־λ��
						finished_bit.fetch_add(1); 
					}
					else
					{
						// ���Ṥ��ֱ���̱߳����ѡ�
						std::unique_lock<std::mutex> lock(wake_mutex);
						wake_condition.wait(lock);
					}
				}
			});

			// �ͷſ���Ȩ
			worker.detach();
		}
	}

	void job_system::execute(const std::function<void()>& job)
	{
		// ���±�־λ
		current_bit += 1;

		// ѭ��ѹ��һ������ֱ���ɹ���
		while(!job_pool.push_back(job))
		{
			// ��������������һ���߳�
			poll();
		}

		// ����һ���̡߳�
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

		// �����߳�����
		const uint32_t group_count = (job_count + group_size - 1) / group_size;

		// ���±�־λ
		current_bit += group_count;

		for(uint32_t group_index = 0; group_index < group_count; ++group_index)
		{
			// Ϊÿ���߳������ɶ�Ӧ������
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

			// ѭ��ѹ��һ������ֱ���ɹ���
			while(!job_pool.push_back(job_group))
			{
				poll();
			}

			// ����һ���߳�
			wake_condition.notify_one();
		}
	}
}