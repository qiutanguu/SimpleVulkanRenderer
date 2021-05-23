#pragma once
#include "../core/core.h"
#include "../asset_system/asset_vmd.h"

namespace flower { namespace graphics{

	glm::mat3 inv_z(const glm::mat3& m)
	{
		const glm::mat3 invZ = glm::scale(glm::mat4(1),glm::vec3(1,1,-1));
		return invZ * m * invZ;
	}

	struct bezier_curve
	{
		inline void initialize(const unsigned char* cp)
		{
			int x0 = cp[0];
			int y0 = cp[4];
			int x1 = cp[8];
			int y1 = cp[12];

			control_point_0 = glm::vec2((float)x0 / 127.0f,(float)y0 / 127.0f);
			control_point_1 = glm::vec2((float)x1 / 127.0f,(float)y1 / 127.0f);
		}

		inline float eval_x(float t) const
		{
			const float t2 = t*t;
			const float t3 = t2*t;
			const float it = 1.0f - t;
			const float it2 = it * it;
			const float it3 = it2 * it;

			const float x[4] = {
				0,
				control_point_0.x,
				control_point_1.x,
				1.0f
			};

			return t3 * x[3] + 3 * t2 * it * x[2] + 3 * t * it2 * x[1] + it3 * x[0];
		}

		float eval_y(float t) const
		{
			const float t2 = t*t;
			const float t3 = t2*t;
			const float it = 1.0f-t;
			const float it2 = it*it;
			const float it3 = it2*it;
			const float y[4] = {
				0,
				control_point_0.y,
				control_point_1.y,
				1,
			};

			return t3*y[3]+3*t2*it*y[2]+3*t*it2*y[1]+it3*y[0];
		}

		glm::vec2 eval(float t) const
		{
			return glm::vec2(eval_x(t), eval_y(t));
		}

		float sample_bezier_x(float time) const
		{
			const float e = 0.00001f;
			float start = 0.0f;
			float stop = 1.0f;
			float t = 0.5f;
			float x = eval_x(t);
			while(std::abs(time-x)>e)
			{
				if(time<x)
				{
					stop = t;
				}
				else
				{
					start = t;
				}
				t = (stop+start)*0.5f;
				x = eval_x(t);
			}

			return t;
		}

		glm::vec2 control_point_0;
		glm::vec2 control_point_1;
	};

	struct animation_key
	{
		void set(const asset::VMDMotion& motion)
		{
			// NOTE: 旋转以匹配Vulkan
			time = int32_t(motion.m_frame);
			translate = motion.m_translate * glm::vec3(1,1,-1);
			const glm::quat q = motion.m_quaternion;

			auto rot0 = glm::mat3_cast(q);
			auto rot1 = inv_z(rot0);
			rotate = glm::quat_cast(rot1);

			txBezier.initialize(&motion.m_interpolation[0]);
			tyBezier.initialize(&motion.m_interpolation[1]);
			tzBezier.initialize(&motion.m_interpolation[2]);
			rotBezier.initialize(&motion.m_interpolation[3]);
		}

		int32_t		time;
		glm::vec3	translate;
		glm::quat	rotate;

		bezier_curve txBezier {};
		bezier_curve tyBezier {};
		bezier_curve tzBezier {};
		bezier_curve rotBezier{};
	};

	struct morph_animation_key
	{
		int32_t	time;
		float	weight;
	};

	struct ik_animation_key
	{
		int32_t	time;
		bool	enable;
	};

	// vmd时间采样
	inline void vmd_animation_evaluate(float time)
	{
		
	}

} }