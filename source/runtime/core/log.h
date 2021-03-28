#pragma once

#include <memory.h>
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace flower
{
	class logger
	{
	private:
		void initialize();

	public:
		logger();
		inline static std::shared_ptr<logger>& get_instance()
		{
			return instance;
		}

		inline std::shared_ptr<spdlog::logger>& get_logger_util()
		{
			return logger_util;
		}

		inline std::shared_ptr<spdlog::logger>& get_logger_io()
		{
			return logger_io;
		}

		inline std::shared_ptr<spdlog::logger>& get_logger_graphics()
		{
			return logger_graphics;
		}

	private:
		static std::shared_ptr<logger> instance;
		std::shared_ptr<spdlog::logger> logger_util;
		std::shared_ptr<spdlog::logger> logger_io;
		std::shared_ptr<spdlog::logger> logger_graphics;
	};
}