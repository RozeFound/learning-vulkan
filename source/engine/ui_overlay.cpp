#include <map>

#include "roboto_regular.h"
#include <imgui.h>

#include "ui_overlay.hpp"

#include "core/pipeline.hpp"

#include "utils/logging.hpp"
#include "utils/primitives.hpp"

namespace engine {

        UI::~UI ( ) {

        logi("Destroying ImGUI");
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

        logi("Destroying UI Pipeline");
        device->get_handle().destroyPipelineLayout(pipeline_layout);
        device->get_handle().destroyDescriptorSetLayout(descriptor_set_layout);
        device->get_handle().destroyPipeline(pipeline);

    }

    void UI::create_handle ( ) {

        descriptor_set_layout = create_descriptor_set_layout();

        auto push_constant_range = vk::PushConstantRange {
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .offset = 0,
            .size = sizeof(glm::vec2) * 2
        };

        auto sample_count = get_max_sample_count(device->get_gpu());
        pipeline_layout = create_pipeline_layout(&descriptor_set_layout, &push_constant_range);
        pipeline = create_pipeline({
            .binding_description = ImVertex::get_binding_description(),
            .attribute_descriptions = ImVertex::get_attribute_descriptions(),
            .rasterization_info = create_rasterization_info(vk::CullModeFlagBits::eNone),
            .multisampling_info = create_multisampling_info(sample_count, false),
            .depth_stencil_info = create_depth_stencil_info(false, false),
            .color_blend_attachment = create_color_blend_attachment(true,
                { vk::BlendFactor::eSrcAlpha,  vk::BlendFactor::eOneMinusSrcAlpha }, vk::BlendOp::eAdd,
                { vk::BlendFactor::eOne,  vk::BlendFactor::eOneMinusSrcAlpha }, vk::BlendOp::eAdd
            ),
            .layout = pipeline_layout,
            .render_pass = render_pass,
            .shader_path = "shaders/imgui"
        });

        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui::GetStyle().WindowRounding = 10.f;

        auto& io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; 

        ImGui_ImplGlfw_InitForVulkan(const_cast<GLFWwindow*>(device->get_window()), false);
        create_font_texture();
        register_callbacks();

        vertex_buffers.resize(image_count);
        index_buffers.resize(image_count);

    }

    void UI::create_font_texture ( ) {

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        auto font_config = ImFontConfig();
        font_config.FontDataOwnedByAtlas = false;
        ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)font, sizeof(font), 16.0f, &font_config);
        io.FontDefault = robotoFont;

        unsigned char* pixels; int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        std::size_t size = width * height * 4;

        auto data = std::vector<std::byte>(reinterpret_cast<std::byte*>(pixels), reinterpret_cast<std::byte*>(pixels) + size);
        font_texture = std::make_unique<Texture>(width, height, data);

    }

    void UI::register_callbacks ( ) {

        auto window = const_cast<GLFWwindow*>(device->get_window());

        glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
        glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
        glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);

    }

    bool UI::update_buffers (uint32_t index) {
        
        auto draw_data = ImGui::GetDrawData();

        if (!draw_data || draw_data->CmdListsCount == 0) return false;

        auto vertex_buffer_size = draw_data->TotalVtxCount * sizeof(ImVertex);
        auto index_buffer_size = draw_data->TotalIdxCount * sizeof(uint16_t);

        auto& vertex_buffer = vertex_buffers.at(index);
        auto& index_buffer = index_buffers.at(index);

        if (!vertex_buffer || (vertex_buffer && vertex_buffer->get_size() < vertex_buffer_size)) 
            vertex_buffer = std::make_unique<Buffer>(vertex_buffer_size, vk::BufferUsageFlagBits::eVertexBuffer, true);
        if (!index_buffer || (index_buffer && index_buffer->get_size() < index_buffer_size)) 
            index_buffer = std::make_unique<Buffer>(index_buffer_size, vk::BufferUsageFlagBits::eIndexBuffer, true);

        auto command_lists = std::vector(draw_data->CmdLists, draw_data->CmdLists + draw_data->CmdListsCount);

        std::ptrdiff_t vertex_offset = 0, index_offset = 0;

        for (const auto& command_list : command_lists) {

            vertex_buffer->write(command_list->VtxBuffer.Data, command_list->VtxBuffer.Size * sizeof(ImVertex), vertex_offset);
            index_buffer->write(command_list->IdxBuffer.Data, command_list->IdxBuffer.Size * sizeof(uint16_t), index_offset);
            
            vertex_offset += command_list->VtxBuffer.Size * sizeof(ImVertex); 
            index_offset += command_list->IdxBuffer.Size * sizeof(uint16_t);
            
        }

        return true;

    }

    void UI::draw (uint32_t index, const vk::CommandBuffer& commands) {

        extern std::map<std::string_view, double> perf_counters;

        SCOPED_PERF_LOG;

        if (!perf_counters.empty()) {

            ImGui::Begin("Perf Counters", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

            for (auto[name, duration] : perf_counters)
                ImGui::Text("%s: %.3fms", name.data(), duration);
                
            ImGui::End();

        }    

        ImGui::Render();

        if (!update_buffers(index)) return;

        commands.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
        commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout, 0, 1, &font_texture->get_descriptor_set(), 0, nullptr);

        auto offsets = std::array<vk::DeviceSize, 1> { }; 
        commands.bindVertexBuffers(0, 1, &vertex_buffers.at(index)->get_handle(), offsets.data());
        commands.bindIndexBuffer(index_buffers.at(index)->get_handle(), 0, vk::IndexType::eUint16);

        auto io = ImGui::GetIO();
        auto draw_data = ImGui::GetDrawData();

        auto scale = glm::vec2(2.0f / draw_data->DisplaySize.x, 2.0f / draw_data->DisplaySize.y);
        auto translate = glm::vec2(-1.f - draw_data->DisplayPos.x * scale.x, -1.f - draw_data->DisplayPos.y * scale.y);
        auto constant = std::array { scale, translate };

        commands.pushConstants(pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::vec2) * 2, &constant);

        auto command_lists = std::vector(draw_data->CmdLists, draw_data->CmdLists + draw_data->CmdListsCount);

        std::size_t index_offset = 0, vertex_offset = 0;

        for (const auto& command_list : command_lists) {

            for (std::size_t i = 0; i < command_list->CmdBuffer.Size; i++) {

                auto cmd = command_list->CmdBuffer[i];

                auto scissor = vk::Rect2D {
                    .offset = { std::max(static_cast<int32_t>(cmd.ClipRect.x), 0), std::max(static_cast<int32_t>(cmd.ClipRect.y), 0) },
                    .extent = { static_cast<uint32_t>(cmd.ClipRect.z - cmd.ClipRect.x), static_cast<uint32_t>(cmd.ClipRect.w - cmd.ClipRect.y) }
                };

                commands.setScissor(0, 1, &scissor);
                commands.drawIndexed(cmd.ElemCount, 1, cmd.IdxOffset + index_offset, cmd.VtxOffset + vertex_offset, 0);

            }

            vertex_offset += command_list->VtxBuffer.Size;
            index_offset += command_list->IdxBuffer.Size;

        }

    }

    void UI::new_frame ( ) {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void UI::end_frame ( ) {
        ImGui::EndFrame();
    }

}