#pragma once
#include <stdint.h>
#include <functional>

namespace flower
{
	/*
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
		  global_timer::reset();
		 	Job();
		 	Job();
		 	Job();
		 	Job();
		 	Job();
		  global_timer::get_timer_milli_sec();
	  �ĳ�Jobsystem���ã����£�
	    global_timer::reset();
		  job_system::execute([]{Job();});
		  job_system::execute([]{Job();});
		  job_system::execute([]{Job();});
		  job_system::execute([]{Job();});
		  job_system::execute([]{Job();});
		  job_system::wait_for_all();
	    global_timer::get_timer_milli_sec();
	  ����������job_system::wait_for_all()��ִ����ϡ�
	*/
	namespace job_system
	{
		// ��ʼ����
		void initialize();

		/// <summary>
		/// ����������첽ִ�С�
		/// </summary>
		/// <param name="job">Ҫ��ӵ�����</param>
		void execute(const std::function<void()>& job);


		// �̷ַ߳�����
		struct dispatch_args
		{
			uint32_t job_id;
			uint32_t group_id;
		};

		/// <summary>
		/// ��һ������ַ�������������в���ִ�С�
		/// </summary>
		/// <param name="job_count"></param>
		/// <param name="group_size"></param>
		/// <param name="job">��Ҫ�ַ�������</param>
		void dispatch(const uint32_t& job_count,const uint32_t& group_size,const std::function<void(dispatch_args)>& job);

		// ����Ƿ����������ڹ�����
		bool is_busy();

		// �ȴ���������ִ����ϡ�
		void wait_for_all();

		void destroy();
	}
}

