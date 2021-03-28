#define SPDLOG_LEVEL_NAMES {  \
	"讯息" /* trace    */,    \
	"调试" /* debug    */,    \
	"通知" /* info     */,    \
	"警告" /* warn     */,    \
	"错误" /* error    */,    \
	"崩溃" /* critical */,    \
	"关闭" /* off      */     \
} 

#include "log.h"
#include "core.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace flower
{
	std::shared_ptr<logger> logger::instance = std::make_shared<logger>();

#ifdef LOG_THREAD_ID
	const auto s_PrintFormat = "%^【%H:%M:%S】【线程ID:%t】【%l】 %n: %v%$";
	const auto s_LogFileFormat = "【%H:%M:%S】【线程ID:%t】【%l】 %n: %v";
#else
	const auto s_PrintFormat = "%^【%H:%M:%S】【%l】 %n: %v%$";
	const auto s_LogFileFormat = "【%H:%M:%S】【%l】 %n: %v";
#endif

	void logger::initialize()
	{
		if(instance != nullptr)
			return;

		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks[0]->set_pattern(s_PrintFormat);
#ifdef LOG_ENABLE_FILE
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("flower.log",true));
		logSinks[1]->set_pattern(s_LogFileFormat);
#endif
		logger_util = std::make_shared<spdlog::logger>("flower",begin(logSinks),end(logSinks));
		spdlog::register_logger(logger_util);
		logger_util->set_level(spdlog::level::trace);
		logger_util->flush_on(spdlog::level::trace);

		logger_io = std::make_shared<spdlog::logger>("io",begin(logSinks),end(logSinks));
		spdlog::register_logger(logger_io);
		logger_io->set_level(spdlog::level::trace);
		logger_io->flush_on(spdlog::level::trace);

		logger_graphics = std::make_shared<spdlog::logger>("graphics",begin(logSinks),end(logSinks));
		spdlog::register_logger(logger_graphics);
		logger_graphics->set_level(spdlog::level::trace);
		logger_graphics->flush_on(spdlog::level::trace);
	}

	logger::logger()
	{
		initialize();
	}
}