#pragma once
#include "core/core.h"

namespace flower
{
	namespace camera_utils
	{
		const float yaw_default = -90.0f;
		const float pitch_default = 0.0f;
		const float speed_default = 2.5f;
		const float sensitivity_default = 0.1f;
		const float zoom_default = 45.0f;

		enum class move_type
		{
			forward,
			backward,
			left,
			right,
		};
	}

	class camera
	{
	public:
        camera(
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
            float yaw = camera_utils::yaw_default, 
            float pitch = camera_utils::pitch_default) : 
            Front(glm::vec3(0.0f, 0.0f, 1.0f)), 
            MovementSpeed(camera_utils::speed_default), 
            MouseSensitivity(camera_utils::sensitivity_default), 
            Zoom(camera_utils::zoom_default)
        {
            Position = position;
            WorldUp = up;
            Yaw = yaw;
            Pitch = pitch;
            updateCameraVectors();
        }


        camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : 
        Front(glm::vec3(0.0f, 0.0f, 1.0f)),
        MovementSpeed(camera_utils::speed_default), 
        MouseSensitivity(camera_utils::sensitivity_default), 
        Zoom(camera_utils::zoom_default)
        {
            Position = glm::vec3(posX, posY, posZ);
            WorldUp = glm::vec3(upX, upY, upZ);
            Yaw = yaw;
            Pitch = pitch;
            updateCameraVectors();
        }

        // ����glm::lookat����ͨ������ŷ���Ƿ�ʽ����View Matrix
        glm::mat4 get_view_matrix()
        {
            return glm::lookAt(Position, Position + Front, Up);
        }

        // ��������¼�
        void ProcessKeyboard(camera_utils::move_type direction, float deltaTime)
        {
            float velocity = MovementSpeed * deltaTime;
            if (direction == camera_utils::move_type::forward)
                Position += Front * velocity;
            if (direction == camera_utils::move_type::backward)
                Position -= Front * velocity;
            if (direction == camera_utils::move_type::left)
                Position -= Right * velocity;
            if (direction == camera_utils::move_type::right)
                Position += Right * velocity;
        }

        // ��������ƶ�
        void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
        {
            xoffset *= MouseSensitivity;
            yoffset *= MouseSensitivity;

            Yaw   += xoffset;
            Pitch += yoffset;

            // ��ֹ��ת
            if (constrainPitch)
            {
                if (Pitch > 89.0f)
                    Pitch = 89.0f;
                if (Pitch < -89.0f)
                    Pitch = -89.0f;
            }

            updateCameraVectors();
        }

        // ��������¼�
        void ProcessMouseScroll(float yoffset)
        {
            Zoom -= (float)yoffset;
            if (Zoom < 1.0f)
                Zoom = 1.0f;
            if (Zoom > 45.0f)
                Zoom = 45.0f; 
        }

        auto GetProjectMatrix(uint32_t app_width,uint32_t app_height)
        {
            auto ret = glm::perspective(glm::radians(Zoom),(float)app_width /(float)app_height, zNear,zFar);
            ret[1][1] *= -1; // Vulkan
            return ret;
        }

	public:
		// �������
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;

		// ŷ����
		float Yaw;
		float Pitch;

        float zNear = 0.1f;
        float zFar = 100.0f;

        // ����
		float MovementSpeed;
		float MouseSensitivity;
		float Zoom;

        // ��һ����������¼�
        bool firstMouse = true;

        float lastX;
        float lastY;

	private:
		void updateCameraVectors()
		{
			glm::vec3 front;
			front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			front.y = sin(glm::radians(Pitch));
			front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			Front = glm::normalize(front);
			Right = glm::normalize(glm::cross(Front, WorldUp)); 
			Up    = glm::normalize(glm::cross(Right, Front));
		}
	};
}