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
    presentation = std::make_unique<class presentation> (kernel->primary_context (), kernel->primary_work_queue ()
#if TARGET_WIN32
        , hi, hw
#elif TARGET_MACOSX
        , v
#elif TARGET_LINUX
        , xc, xw
#endif
    );

    presentation->create ();

    state.compute_size = calculate_compute_size ();
    state.canvas_viewport = calculate_canvas_viewport ();
}

void vk::create_systems (const std::function<void ()>& z_imgui_fn) {

    compute_target = std::make_unique<class compute_target> (
        kernel->primary_context (),
        kernel->primary_work_queue (),
        sge::app::get_content (),
        [this]() { return state.compute_size; }
        );
    compute_target->create ();

    canvas_render = std::make_unique<class canvas_render> (
        kernel->primary_context (),
        kernel->primary_work_queue (),
        *presentation.get (),
        [this]() { return compute_target->get_pre_render_texture ().descriptor; },
        [this]() {
            return state.canvas_viewport;
        }
    );
    canvas_render->create ();

    // ImGUI
    imgui = std::make_unique<class imgui> (
        kernel->primary_context (),
        kernel->primary_work_queue (),
        *presentation.get (),
        z_imgui_fn);
	imgui->create ();

}
void vk::destroy () {
    imgui->destroy ();
    imgui.reset ();

	canvas_render->destroy ();
	canvas_render.reset ();

    compute_target->destroy ();
    compute_target.reset ();

	presentation->destroy ();
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

void vk::update (bool& push_flag, std::vector<bool>& ubo_flags, std::vector<std::optional<dataspan>>& sbo_flags, float dt) {

	bool refresh = false;

    VkResult* failure = nullptr;
    std::variant<VkResult, image_index> image_index_opt;
    if (presentation->in_limbo ()) {
        refresh = true;
    }
    else {
        image_index_opt = presentation->next_image ();
        failure = std::get_if<VkResult> (&image_index_opt);
    }

	if (failure) {
        switch (*failure) {
            case VK_ERROR_OUT_OF_DATE_KHR:
                refresh = true;
                break;
            case VK_ERROR_SURFACE_LOST_KHR:
                refresh = false;
                break;
            default: break;
        }
	}

    if (!failure && !refresh) {
        if (!utils::equal (calculate_compute_size (), state.compute_size)) { refresh = true; }
        if (!utils::equal (calculate_canvas_viewport (), state.canvas_viewport)) { refresh = true; }

        compute_target->update (push_flag, ubo_flags, sbo_flags);
    }


	if (!failure && !refresh) {


        // system enqueues
        compute_target->enqueue ();

        auto image_index = std::get<sge::vk::image_index> (image_index_opt);
        if (state.imgui_on) {
            imgui->enqueue (image_index);
        }

		std::vector<VkSemaphore> wait_on = { presentation->image_available () }; // + user compute complete
		std::vector<VkPipelineStageFlags> stage_flags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		compute_target->append_pre_render_submissions (wait_on, stage_flags);

        // todo: switch to using: https://www.khronos.org/blog/vulkan-timeline-semaphores
        submit (
        	canvas_render->get_command_buffer (image_index),
            canvas_render->get_queue (),
            wait_on,
            stage_flags,
            canvas_render->get_render_finished ());

        VkSemaphore render_finished[] = { canvas_render->get_render_finished () };

        if (state.imgui_on) {
            submit (
                imgui->get_command_buffer (image_index),
                imgui->get_queue (),
                canvas_render->get_render_finished (),
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                imgui->get_render_finished ());

            render_finished[0] = imgui->get_render_finished ();
        }

		VkSwapchainKHR swap_chain[] = { presentation->swapchain () };
		auto present_info = utils::init_VkPresentInfoKHR ();
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = render_finished;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swap_chain;
		present_info.pImageIndices = &image_index;

		auto p_queue = kernel->get_queue (kernel->primary_work_queue ());
		VkResult result = vkQueuePresentKHR (p_queue, &present_info);

		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
            vk_assert (vkQueueWaitIdle (p_queue));
            refresh = true;
        }
		else {
			vk_assert (result);
			vk_assert (vkQueueWaitIdle (p_queue));
		}
    }

    vk_assert (vkDeviceWaitIdle (kernel->primary_context ().logical_device));

    // RECREATE
    if (refresh) {
		presentation->refresh ();

        if (presentation->in_limbo ())
            return;

        state.compute_size = calculate_compute_size ();
        state.canvas_viewport = calculate_canvas_viewport ();

        compute_target->recreate ();
		canvas_render->refresh_full ();
		imgui->refresh ();
    }
    else {
        compute_target->end_of_frame ();
    }
}


};
