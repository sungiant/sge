#include "sge_vk.hh"

#include "sge_vk_context.hh"
#include "imgui_ext.hh"

namespace sge::vk {

VkViewport vk::calculate_canvas_viewport () {
    const auto e = presentation->extent ();
    if (state.imgui_on) {
        const int imgui_main_menu_bar_height = ::imgui::ext::guess_main_menu_bar_height();
        const VkViewport vp = utils::init_VkViewport (0, imgui_main_menu_bar_height, (float) e.width, (float) e.height - imgui_main_menu_bar_height, 0.0f, 1.0f);
        return vp;
    }
    else {
        return utils::init_VkViewport (0, 0, (float) e.width, (float) e.height, 0.0f, 1.0f);
    }
}

VkExtent2D vk::calculate_compute_size () {
    const auto e = presentation->extent();
    if (state.imgui_on) {
        const int imgui_main_menu_bar_height = ::imgui::ext::guess_main_menu_bar_height();
        return VkExtent2D { e.width, e.height - imgui_main_menu_bar_height };
    }
    else return VkExtent2D { e.width, e.height };
}

#if TARGET_WIN32
void vk::create (HINSTANCE hi, HWND hw, int w, int h) {
#elif TARGET_MACOSX
void vk::create (void* v, int w, int h) {
#elif TARGET_LINUX
void vk::create (xcb_connection_t* xc, xcb_window_t xw, int w, int h) {
#else
void vk::create (int w, int h) {
#endif

    // Create kernal
    kernel = std::make_unique<class kernel> ();
    kernel->create ();

    // Create presentation
    presentation = std::make_unique<class presentation> (kernel->primary_context (), kernel->primary_graphics_queue_id ()
#if TARGET_WIN32
        , hi, hw
#elif TARGET_MACOSX
        , v
#elif TARGET_LINUX
        , xc, xw
#endif
    );

    presentation->create_resources (presentation::all_resources);

    state.compute_size = calculate_compute_size ();
    state.canvas_viewport = calculate_canvas_viewport ();
}

void vk::create_systems (const std::function<void ()>& z_imgui_fn) {

    compute_target = std::make_unique<class compute_target> (
        kernel->primary_context (),
        kernel->primary_compute_queue_id (),
        sge::app::get_content (),
        [this]() { return state.compute_size; }
        );
    compute_target->create ();

    canvas_render = std::make_unique<class canvas_render> (
        kernel->primary_context (),
        kernel->primary_graphics_queue_id (),
        *presentation.get (),
        [this]() { return compute_target->get_pre_render_texture ().descriptor; },
        [this]() {
            return state.canvas_viewport;
        }
    );
    canvas_render->create_resources (canvas_render::all_resources);

    // ImGUI
    imgui = std::make_unique<class imgui> (
        kernel->primary_context (),
        kernel->primary_graphics_queue_id (),
        *presentation.get (),
        z_imgui_fn);
    imgui->create_resources (imgui::static_resources);

}
void vk::destroy () {
    imgui->destroy_resources (imgui::all_resources);
    imgui.reset ();

    canvas_render->destroy_resources (canvas_render::all_resources);
    canvas_render.reset ();

    compute_target->destroy ();
    compute_target.reset ();

    presentation->destroy_resources (presentation::all_resources);
    presentation.reset ();

    kernel->destroy ();
    kernel.reset ();
}

void submit (const VkCommandBuffer& command_buffer, const VkQueue& queue, const std::vector<VkSemaphore>& wait_on, const std::vector <VkPipelineStageFlags>& pipelineStageFlags, const std::vector <VkSemaphore>& signals) {
    auto submitInfo = utils::init_VkSubmitInfo();
    submitInfo.waitSemaphoreCount = (uint32_t) wait_on.size ();
    submitInfo.pWaitSemaphores = wait_on.data ();
    submitInfo.pWaitDstStageMask = pipelineStageFlags.data ();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer;
    submitInfo.signalSemaphoreCount = (uint32_t) signals.size ();
    submitInfo.pSignalSemaphores = signals.data ();
    vk_assert (vkQueueSubmit (queue, 1, &submitInfo, VK_NULL_HANDLE));
}

void submit (const VkCommandBuffer& command_buffer, const VkQueue& queue, const VkSemaphore wait_on, const VkPipelineStageFlags stageFlag, const VkSemaphore signal) {
    const std::vector<VkSemaphore> wx = { wait_on };
    const std::vector <VkPipelineStageFlags> sf = { stageFlag };
    const std::vector <VkSemaphore> sx = { signal };
    submit (command_buffer, queue, wx, sf, sx);
}

void submit (const VkCommandBuffer& command_buffer, const VkQueue& queue, const std::vector<VkSemaphore>& wait_on, const std::vector <VkPipelineStageFlags>& stageFlags, const VkSemaphore signal) {
    const std::vector <VkSemaphore> sx = { signal };
    submit (command_buffer, queue, wait_on, stageFlags, sx);
}

VkSemaphore vk::submit_all (image_index image_index) {
    // system enqueues
    compute_target->enqueue ();

    if (state.imgui_on) {
        imgui->record (image_index);
    }

    std::vector<VkSemaphore> wait_on = { presentation->image_available (), compute_target->get_compute_finished () };
    std::vector<VkPipelineStageFlags> stage_flags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT  };

    assert (wait_on.size () == stage_flags.size ());

    // todo: switch to using: https://www.khronos.org/blog/vulkan-timeline-semaphores
    submit (
        canvas_render->get_command_buffer (image_index),
        canvas_render->get_queue (),
        wait_on,
        stage_flags,
        canvas_render->get_render_finished ());

    if (state.imgui_on) {
        submit (
            imgui->get_command_buffer (image_index),
            imgui->get_queue (),
            canvas_render->get_render_finished (),
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            imgui->get_render_finished ());
        return imgui->get_render_finished ();
    }
    else {
        return canvas_render->get_render_finished ();
    }
}

void vk::update (bool& push_flag, std::vector<bool>& ubo_flags, std::vector<std::optional<dataspan>>& sbo_flags, float dt) {
    const auto surface_status = presentation->check_surface_status ();
    const bool surface_ok = surface_status == presentation::surface_status::OK;
    const bool surface_minimised = surface_status == presentation::surface_status::ZERO;

    std::variant<presentation::swapchain_status, image_index> swapchain_status = presentation::swapchain_status::FATAL;

    if (surface_ok) {
        swapchain_status = presentation->next_image ();

        {
            const presentation::swapchain_status* swapchain_issue = std::get_if<presentation::swapchain_status> (&swapchain_status);
            if (swapchain_issue && (*swapchain_issue == presentation::swapchain_status::OUT_OF_DATE || *swapchain_issue == presentation::swapchain_status::SUBOPTIMAL)) {
                // an issue we can deal with, fix it and crack on
                presentation->destroy_resources (presentation::transient_resources);
                presentation->create_resources (presentation::transient_resources);

                swapchain_status = presentation->next_image ();
                assert (!std::get_if<presentation::swapchain_status> (&swapchain_status)); // make sure we fixed the issue
            }
            else if (swapchain_issue) {
                assert (false);
                return; // there's an issue we the swapchain that we currently can't deal with. not expected as surface status check should catch these issues first.
            }
        }

        assert (std::get_if<image_index> (&swapchain_status)); // at this point the swapchain should be ok
    }
    else if (!surface_minimised) return; // lost

    const bool swapchain_ok = std::get_if<image_index> (&swapchain_status);

    if (surface_ok && swapchain_ok) {

        const VkExtent2D required_compute_size = calculate_compute_size ();
        const VkViewport required_canvas_viewport = calculate_canvas_viewport ();
        const bool compute_size_needs_refresh = !utils::equal (required_compute_size, state.compute_size);
        const bool canvas_viewport_needs_refresh = !utils::equal (required_canvas_viewport, state.canvas_viewport) || compute_size_needs_refresh;

        if (canvas_viewport_needs_refresh)  canvas_render->destroy_resources (canvas_render::transient_resources);
        if (compute_size_needs_refresh)     compute_target->destroy_r ();

        state.compute_size = required_compute_size;
        state.canvas_viewport = required_canvas_viewport;

        if (compute_size_needs_refresh)     compute_target->create_r ();
        if (canvas_viewport_needs_refresh)  canvas_render->create_resources (canvas_render::transient_resources);
    }

    // pre-update
    compute_target->update (push_flag, ubo_flags, sbo_flags);

    if (surface_ok && swapchain_ok) {
        const uint32_t image_index = std::get<sge::vk::image_index> (swapchain_status);

        const VkSemaphore all_done = submit_all (image_index);

        const auto present_info = utils::init_VkPresentInfoKHR (all_done, presentation->swapchain (), image_index);
        const VkResult result = vkQueuePresentKHR (kernel->primary_graphics_queue (), &present_info); // ignore the result as any error we'll deal with next frame.
    }

    // post-update
    compute_target->end_of_frame ();

    vk_assert (vkDeviceWaitIdle (kernel->primary_context ().logical_device));
}

void vk::debug_ui () {

    ImGui::Text ("Compute target size: %dx%d", compute_target->current_width (), compute_target->current_height ());

    ImGui::Separator ();

    kernel->debug_ui ();

    ImGui::Separator ();
    
    imgui->debug_ui ();
}

};
