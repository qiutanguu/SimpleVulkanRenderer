#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "ui_context.h"
#include "ui_shader.h"
#include "core/core.h"
#include "../vk/vk_command_buffer.h"
#include "../vk/vk_device.h"
#include "../vk/vk_common.h"

namespace flower{ namespace graphics{

    void ui_context::initialize(uint32_t back_buffer_counts)
    {
		create_descriptor_pool();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance;
        init_info.PhysicalDevice = device->physical_device;
        init_info.Device = *device;
        init_info.QueueFamily = device->find_queue_families().graphics_family;
        init_info.Queue = device->graphics_queue;
        init_info.PipelineCache = pipeline_cache;
        init_info.DescriptorPool = descriptor_pool;
        init_info.Allocator = nullptr;
        init_info.MinImageCount = min_image_count;
        init_info.ImageCount = back_buffer_counts;
        init_info.CheckVkResultFn = vk_check;

		{
			auto cmd_buffer = vk_command_buffer::create(*device,command_pool);
			cmd_buffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
				ImGui_ImplVulkan_CreateFontsTexture(cmd_buffer->get_instance());
			cmd_buffer->end();

			VkSubmitInfo end_info = {};
			end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			end_info.commandBufferCount = 1;
			end_info.pCommandBuffers = &cmd_buffer->get_instance();
			vk_check(vkQueueSubmit(device->graphics_queue,1,&end_info,VK_NULL_HANDLE));

			vk_check(vkDeviceWaitIdle(*device));
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
    }

	void ui_context::draw_frame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if(show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window",&show_demo_window);     
			ImGui::Checkbox("Another Window",&show_another_window);

			ImGui::SliderFloat("float",&f,0.0f,1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color",(float*)&clear_color); // Edit 3 floats representing a color

			if(ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d",counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",1000.0f/ImGui::GetIO().Framerate,ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if(show_another_window)
		{
			ImGui::Begin("Another Window",&show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if(ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
	}

	void ui_context::destroy_descriptor_pool()
	{
		vkDestroyDescriptorPool(*device,descriptor_pool,nullptr);
	}

	void ui_context::create_descriptor_pool()
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000*IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		vk_check(vkCreateDescriptorPool(*device,&pool_info,nullptr,&descriptor_pool));
	}

	void ui_context::destroy()
	{

		vk_check(vkDeviceWaitIdle(*device));
		destroy_descriptor_pool();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

} }