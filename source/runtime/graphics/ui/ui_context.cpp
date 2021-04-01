#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "ui_context.h"
#include "ui_shader.h"
#include "core/core.h"

namespace flower{ namespace graphics{

    void ui_context::initialize()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance;
        init_info.PhysicalDevice = gpu;
        init_info.Device = device;
        init_info.QueueFamily = device.find_queue_families().graphics_family;
        init_info.Queue = device.graphics_queue;
        init_info.PipelineCache = pipeline_cache;
        init_info.DescriptorPool = descriptor_pool;
        init_info.Allocator = nullptr;
        init_info.MinImageCount = g_MinImageCount;
        init_info.ImageCount = wd->ImageCount;
        init_info.CheckVkResultFn = check_vk_result;
    }

} }