#pragma once

namespace flower
{
	class iruntime_module
	{
	public:
		iruntime_module() = default;
		virtual ~iruntime_module(){}

		virtual void initialize() = 0;
		virtual void after_initialize() = 0;

		virtual void tick(float time, float delta_time) = 0;

		virtual void before_destroy() = 0;
		virtual void destroy() = 0;

		uint32_t execute_index = 0;
	};
}