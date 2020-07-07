#include "sge_vk.hh"

#include "sge_vk_context.hh"
#include "imgui_ext.hh"

namespace sge::vk {

VkViewport vk::calculate_viewport (const class presentation& p) {
    if (imgui_on) {
        const int imgui_main_menu_bar_height = ::imgui::ext::guess_main_menu_bar_height();
        return utils::init_VkViewport (0, imgui_main_menu_bar_height, (float) p.extent ().width, (float) p.extent ().height - imgui_main_menu_bar_height, 0.0f, 1.0f);
    }
    else {
        return utils::init_VkViewport (0, 0, (float) p.extent ().width, (float) p.extent ().height, 0.0f, 1.0f);
    }
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
}

void vk::create_systems (const std::function<void ()>& z_imgui_fn) {

    compute_target = std::make_unique<class compute_target> (
        kernel->primary_context (),
        kernel->primary_work_queue (),
        *presentation.get (),
        sge::app::get_content ());
    compute_target->create ();

    fullscreen_render = std::make_unique<class fullscreen_render> (
        kernel->primary_context (),
        kernel->primary_work_queue (),
        *presentation.get (),
        [this]() {
            return compute_target->get_pre_render_texture ().descriptor;
        });
    fullscreen_render->set_custom_viewport_fn(std::bind (&vk::calculate_viewport, this, std::placeholders::_1));
    fullscreen_render->create ();

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

	fullscreen_render->destroy ();
	fullscreen_render.reset ();

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

    compute_target->update (push_flag, ubo_flags, sbo_flags);
    compute_target->enqueue ();

	auto image_index_opt = presentation->next_image ();

    VkResult* failure = std::get_if<VkResult> (&image_index_opt);

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
	else {

        if (fullscreen_render->need_command_buffers_refresh()) {
            fullscreen_render->refresh_command_buffers();
        }
        
        auto image_index = std::get<sge::vk::image_index> (image_index_opt);
        if (imgui_on) {
            imgui->enqueue (image_index);
        }

		std::vector<VkSemaphore> wait_on = { presentation->image_available () }; // + user compute complete
		std::vector<VkPipelineStageFlags> stage_flags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		compute_target->append_pre_render_submissions (wait_on, stage_flags);

        // todo: switch to using: https://www.khronos.org/blog/vulkan-timeline-semaphores
        submit (
        	fullscreen_render->get_command_buffer (image_index),
            fullscreen_render->get_queue (),
            wait_on,
            stage_flags,
            fullscreen_render->get_render_finished ());

        VkSemaphore render_finished[] = { fullscreen_render->get_render_finished () };

        if (imgui_on) {
            submit (
                imgui->get_command_buffer (image_index),
                imgui->get_queue (),
                fullscreen_render->get_render_finished (),
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
        compute_target->recreate ();
		fullscreen_render->refresh_full ();
		imgui->refresh ();
    }
    else {
        
        compute_target->end_of_frame ();
    }
}


};
