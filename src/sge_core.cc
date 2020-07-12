#include "sge_core.hh"

#include "sge_app_interface.hh"

namespace sge::core {

guid guid::empty = {};

bool guid::operator == (const guid& other) const {
    for (int i = 0; i < 16; ++i) {
        if (data[i] != other.data[i])
            return false;
    }
    return true;
}

bool guid::operator < (const guid& other) const {
    for (int i = 0; i < 16; ++i) {
        if (data[i] < other.data[i])
            return true;
    }
    return false;
}

guid guid::random () {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size> {};
    std::generate (std::begin (seed_data), std::end (seed_data), std::ref (rd));
    std::seed_seq seq (std::begin (seed_data), std::end (seed_data));
    std::mt19937 generator (seq);
    std::uniform_int_distribution<uint32_t>  distribution;
    guid g;
    for (int i = 0; i < 16; i += 4)
        *reinterpret_cast<uint32_t*> (g.data + i) = distribution (generator);

    // Mask in Variant 1-0 in Bit[7..6]
    g.data[8] &= 0x3F;
    g.data[8] |= 0x80;
    // Mask in Version 4 (random based Guid) in Bits[15..13]
    g.data[7] &= 0x0F;
    g.data[7] |= 0x40;
    return g; // See: https://github.com/mono/mono/blob/master/mcs/class/corlib/System/Guid.cs
}

std::string guid::str () const {
    char b[256];
    sprintf (b, "(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)",
        data[0], data[1], data[2], data[3],
        data[4], data[5], data[6], data[7],
        data[8], data[9], data[10], data[11],
        data[12], data[13], data[14], data[15]);
    std::string r (b);
    return r;
}


//--------------------------------------------------------------------------------------------------------------------//

api_impl::api_impl (const core::engine_state& z_state, core::engine_tasks& z_tasks, std::unordered_map<size_t, std::unique_ptr<runtime::extension>>& z_exts)
    : engine_state (z_state)
    , engine_tasks (z_tasks)
    , engine_extensions (z_exts)
{}

bool api_impl::system__get_state_bool (runtime::system_bool_state z) const {
    switch (z){

        case runtime::system_bool_state::fullscreen: return engine_state.host.is_fullscreen;
        case runtime::system_bool_state::imgui: return engine_state.graphics.state.imgui_on;
        default: assert (false); return false;
    }
}
int api_impl::system__get_state_int (runtime::system_int_state z) const {
    switch (z) {
        case runtime::system_int_state::max_canvas_width: return engine_state.client.max_container_width;
        case runtime::system_int_state::max_canvas_height: return engine_state.client.max_container_height;
        case runtime::system_int_state::canvas_offset_x: return engine_state.graphics.get_user_viewport_x ();
        case runtime::system_int_state::canvas_offset_y: return engine_state.graphics.get_user_viewport_y();
        case runtime::system_int_state::canvas_width: return engine_state.graphics.get_user_viewport_width ();
        case runtime::system_int_state::canvas_height: return engine_state.graphics.get_user_viewport_height ();
        default: assert (false); return 0;
    }
}
const char* api_impl::system__get_state_string (runtime::system_string_state z) const {
    switch (z) {
        case runtime::system_string_state::title: return engine_state.host.window_title.c_str ();
        case runtime::system_string_state::gpu_name: return engine_state.graphics.kernel->get_physical_device_name();
        case runtime::system_string_state::engine_version: {
            return engine_state.version.c_str ();
        }
        default: assert (false); return nullptr;
    }
}

bool api_impl::system__did_container_just_change () const { return engine_state.host.container_just_changed; }

uint32_t api_impl::timer__get_fps () const { return engine_state.instrumentation.lastFPS; }
float api_impl::timer__get_delta () const { return engine_state.instrumentation.frameTimer; }
float api_impl::timer__get_time () const { return engine_state.instrumentation.totalTimer; }

void api_impl::input__keyboard_pressed_characters (uint32_t* z_size, wchar_t* z_keys) const {
    const int first = static_cast<int>(input_control_identifier::kc_0);
    const int last = static_cast<int>(input_control_identifier::kc_9);
    // size query.
    if (z_keys == nullptr) {
        *z_size = 0;
        for (int i = first; i <= last; ++i) {
            const auto id = static_cast<input_control_identifier> (i);
            if (engine_state.input.find (id) != engine_state.input.end ())
                (*z_size)++;
        }
        return;
    }
    // full call.
    uint32_t idx = 0;
    for (int i = first; i <= last; ++i) {
        const auto id = static_cast<input_control_identifier> (i);
        const auto o = (engine_state.input.find (id) != engine_state.input.end ())
            ? std::optional<input_character_control>(std::get<input_character_control> (engine_state.input.at (id)))
            : std::nullopt;
        if (o.has_value ())
            z_keys[idx++] = o.value ();
    }
    assert (idx == *z_size);
}

std::optional<runtime::keyboard_key> convert_to_keyboard_key (input_control_identifier z) {
    #define CASE1(x) { case input_control_identifier::kb_ ## x: return runtime::keyboard_key::x; }
    #define CASE2(x, y) { case input_control_identifier::kb_ ## x: return runtime::keyboard_key::y; }
    switch (z) {
        CASE1(escape); CASE1(enter); CASE1(spacebar); CASE1(shift); CASE1(control); CASE1(alt); CASE1(backspace); CASE1(tab);
        CASE1(ins); CASE1(del); CASE1(home); CASE1(end); CASE1(page_up); CASE1(page_down); CASE1(right_click); CASE1(prt_sc); CASE1(pause);
        CASE1(up); CASE1(down); CASE1(left); CASE1(right);
        CASE1(a); CASE1(b); CASE1(c); CASE1(d); CASE1(e); CASE1(f); CASE1(g); CASE1(h); CASE1(i); CASE1(j); CASE1(k); CASE1(l); CASE1(m);
        CASE1(n); CASE1(o); CASE1(p); CASE1(q); CASE1(r); CASE1(s); CASE1(t); CASE1(u); CASE1(v); CASE1(w); CASE1(x); CASE1(y); CASE1(z);
        CASE2(0, zero); CASE2(1, one); CASE2(2, two); CASE2(3, three); CASE2(4, four); CASE2(5, five); CASE2(6, six); CASE2(7, seven); CASE2(8, eight); CASE2(9, nine);
        CASE1(plus); CASE1(minus); CASE1(comma); CASE1(period);
        CASE1(windows); CASE1(cmd);
        CASE1(f1); CASE1(f2); CASE1(f3); CASE1(f4); CASE1(f5); CASE1(f6); CASE1(f7); CASE1(f8); CASE1(f9); CASE1(f10); CASE1(f11); CASE1(f12);
        CASE1(numpad_0); CASE1(numpad_1); CASE1(numpad_2); CASE1(numpad_3); CASE1(numpad_4); CASE1(numpad_5); CASE1(numpad_6); CASE1(numpad_7); CASE1(numpad_8); CASE1(numpad_9);
        CASE1(numpad_decimal); CASE1(numpad_divide); CASE1(numpad_multiply); CASE1(numpad_subtract); CASE1(numpad_add); CASE1(numpad_enter); CASE1(numpad_equals);
        default: return std::nullopt;
    }
    #undef CASE2
    #undef CASE1
}

std::optional<runtime::gamepad_button> convert_to_gamepad_button (input_control_identifier z) {
    #define CASE(x) { case input_control_identifier::gb_ ## x ## _0: return runtime::gamepad_button::x; }
    switch (z) {
        CASE(dpad_up); CASE(dpad_down); CASE(dpad_left); CASE(dpad_right);
        CASE(start); CASE(center); CASE(back);
        CASE(left_thumb); CASE(right_thumb); CASE(left_shoulder); CASE(right_shoulder);
        CASE(a); CASE(b); CASE(x); CASE(y);
        default: return std::nullopt;
    }
    #undef CASE
}

std::optional<runtime::gamepad_axis> convert_to_gamepad_axis (input_control_identifier z) {
    #define CASE(x, y) { case input_control_identifier::ga_ ## x ## _0: return runtime::gamepad_axis::y; }
    switch (z) {
        CASE(left_stick_x, left_stick_horizontal);
        CASE(left_stick_y, left_stick_vertical);
        CASE(right_stick_x, right_stick_horizontal);
        CASE(right_stick_y, right_stick_vertical);
        CASE(left_trigger, left_trigger);
        CASE(right_trigger, right_trigger);
        default: return std::nullopt;
    }
    #undef CASE
}

void api_impl::input__keyboard_pressed_keys (uint32_t* z_size, runtime::keyboard_key* z_keys) const {
    const int first = static_cast<int>(input_control_identifier::kb_escape);
    const int last = static_cast<int>(input_control_identifier::kb_numpad_equals);
    // size query.
    if (z_keys == nullptr) {
        *z_size = 0;
        for (int i = first; i <= last; ++i) {
            const auto id = static_cast<input_control_identifier> (i);
            if (engine_state.input.find (id) != engine_state.input.end ())
                (*z_size)++;
        }
        return;
    }
    // full call.
    uint32_t idx = 0;
    for (int i = first; i <= last; ++i) {
        const auto id = static_cast<input_control_identifier> (i);
        const auto o = (engine_state.input.find (id) != engine_state.input.end ())
            ? std::optional<input_binary_control>(std::get<input_binary_control> (engine_state.input.at (id)))
            : std::nullopt;
        auto v = convert_to_keyboard_key (id);
        if (o.has_value () && o.value () && v.has_value())
            z_keys[idx++] = v.value();
    }
    assert (idx == *z_size);
}

void api_impl::input__keyboard_pressed_locks (uint32_t* z_size, runtime::keyboard_lock* z_keys) const {
    // size query.
    if (z_keys == nullptr) {
        *z_size = 0;
        if ((engine_state.input.find (input_control_identifier::kq_caps_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_caps_lk)).second : false) (*z_size)++;
        if ((engine_state.input.find (input_control_identifier::kq_scr_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_scr_lk)).second : false) (*z_size)++;
        if ((engine_state.input.find (input_control_identifier::kq_num_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_num_lk)).second : false) (*z_size)++;
        return;
    }
    // full call.
    int idx = 0;
    if ((engine_state.input.find (input_control_identifier::kq_caps_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_caps_lk)).second : false) z_keys[idx++] = runtime::keyboard_lock::caps_lk;
    if ((engine_state.input.find (input_control_identifier::kq_scr_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_scr_lk)).second : false) z_keys[idx++] = runtime::keyboard_lock::scr_lk;
    if ((engine_state.input.find (input_control_identifier::kq_num_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_num_lk)).second : false) z_keys[idx++] = runtime::keyboard_lock::num_lk;
    assert (idx == *z_size);
}

void api_impl::input__keyboard_locked_locks (uint32_t* z_size, runtime::keyboard_lock* z_keys) const {
    // size query.
    if (z_keys == nullptr) {
        *z_size = 0;
        if ((engine_state.input.find (input_control_identifier::kq_caps_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_caps_lk)).first : false) (*z_size)++;
        if ((engine_state.input.find (input_control_identifier::kq_scr_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_scr_lk)).first : false) (*z_size)++;
        if ((engine_state.input.find (input_control_identifier::kq_num_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_num_lk)).first : false) (*z_size)++;
        return;
    }
    // full call.
    int idx = 0;
    if ((engine_state.input.find (input_control_identifier::kq_caps_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_caps_lk)).first : false) z_keys[idx++] = runtime::keyboard_lock::caps_lk;
    if ((engine_state.input.find (input_control_identifier::kq_scr_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_scr_lk)).first : false) z_keys[idx++] = runtime::keyboard_lock::scr_lk;
    if ((engine_state.input.find (input_control_identifier::kq_num_lk) != engine_state.input.end ()) ? std::get<input_quaternary_control> (engine_state.input.at (input_control_identifier::kq_num_lk)).first : false) z_keys[idx++] = runtime::keyboard_lock::num_lk;
    assert (idx == *z_size);
}


void api_impl::input__mouse_pressed_buttons (uint32_t* z_size, runtime::mouse_button* z_keys) const {
    // size query.
    if (z_keys == nullptr) {
        *z_size = 0;
        if (engine_state.input.find (input_control_identifier::mb_left)   != engine_state.input.end ()) (*z_size)++;
        if (engine_state.input.find (input_control_identifier::mb_middle) != engine_state.input.end ()) (*z_size)++;
        if (engine_state.input.find (input_control_identifier::mb_right)  != engine_state.input.end ()) (*z_size)++;
        return;
    }
    // full call.
    int idx = 0;
    if (engine_state.input.find (input_control_identifier::mb_left)   != engine_state.input.end ()) z_keys[idx++] = runtime::mouse_button::left;
    if (engine_state.input.find (input_control_identifier::mb_middle) != engine_state.input.end ()) z_keys[idx++] = runtime::mouse_button::middle;
    if (engine_state.input.find (input_control_identifier::mb_right)  != engine_state.input.end ()) z_keys[idx++] = runtime::mouse_button::right;
    assert (idx == *z_size);
}

void api_impl::input__mouse_position (int* x, int* y) const {
    sge::math::point2 p = (engine_state.input.find (input_control_identifier::mp_position) != engine_state.input.end ())
        ? std::get<input_point_control> (engine_state.input.at (input_control_identifier::mp_position))
        : sge::math::point2 { 0, 0 };
    *x = p.x;
    *y = p.y;
}

void api_impl::input__mouse_scrollwheel (int* z) const {
    *z = (engine_state.input.find (input_control_identifier::md_scrollwheel) != engine_state.input.end ())
        ? std::get<input_digital_control> (engine_state.input.at (input_control_identifier::md_scrollwheel))
        : 0;
}

void api_impl::input__gamepad_pressed_buttons (uint32_t* z_size, runtime::gamepad_button* z_keys) const {

    if (z_keys == nullptr) {
        *z_size = 0;
    }
    const int first = static_cast<int>(input_control_identifier::gb_dpad_up_0);
    const int last = static_cast<int>(input_control_identifier::gb_y_0); // right now the runtime api doesn't support multiple gamepads

    uint32_t idx = 0;
    for (int i = first; i <= last; ++i) {
        const auto id = static_cast<input_control_identifier> (i);
        const auto o = (engine_state.input.find (id) != engine_state.input.end ())
            ? std::optional<input_binary_control>(std::get<input_binary_control> (engine_state.input.at (id)))
            : std::nullopt;
        auto v = convert_to_gamepad_button (id);
        if (o.has_value () && o.value () && v.has_value()) {
            if (z_keys == nullptr)
                (*z_size)++;
            else
                z_keys[idx++] = v.value();
        }
    }

    assert (idx == *z_size || !z_keys);

}

void api_impl::input__gamepad_analogue_axes (uint32_t* z_size, runtime::gamepad_axis* z_keys, float* z_values) const {
    if (z_keys == nullptr || z_values == nullptr) {
        *z_size = 0;
    }
    const int first = static_cast<int>(input_control_identifier::ga_left_stick_x_0);
    const int last = static_cast<int>(input_control_identifier::ga_right_trigger_0); // right now the runtime api doesn't support multiple gamepads

    uint32_t idx = 0;
    for (int i = first; i <= last; ++i) {
        const auto id = static_cast<input_control_identifier> (i);
        const auto o = (engine_state.input.find (id) != engine_state.input.end ())
            ? std::optional<input_analogue_control>(std::get<input_analogue_control> (engine_state.input.at (id)))
            : std::nullopt;
        auto v = convert_to_gamepad_axis (id);
        if (o.has_value () && v.has_value()) {
            if (z_keys == nullptr || z_values == nullptr)
                (*z_size)++;
            else {
                z_keys[idx] = v.value();
                z_values[idx] = o.value ();
                idx++;
            }
        }
    }
    assert (idx == *z_size || !z_keys || !z_values);
}

void api_impl::input__touches (uint32_t* z_size, uint32_t*, int*, int*) const {
    *z_size = 0;
}


//--------------------------------------------------------------------------------------------------------------------//
// Non const functions - these don't do anything directly, the just add stuff to the engine task queue.
//--------------------------------------------------------------------------------------------------------------------//

void api_impl::system__request_shutdown () {
    engine_tasks.shutdown_request = std::monostate {};
}

void api_impl::system__toggle_state_bool (runtime::system_bool_state z) {
    bool v = system__get_state_bool (z);
    system__set_state_bool (z, !v);
}
void api_impl::system__set_state_bool (runtime::system_bool_state z, bool v) {
    switch (z){
        case runtime::system_bool_state::fullscreen: engine_tasks.change_fullscreen_enabled = v; break;
        case runtime::system_bool_state::imgui: engine_tasks.change_imgui_enabled = v; break;
        default: break;
    }
}
void api_impl::system__set_state_int (runtime::system_int_state z, int v) {
    switch (z){
        case runtime::system_int_state::canvas_width: engine_tasks.change_canvas_width = v; break;
        case runtime::system_int_state::canvas_height: engine_tasks.change_canvas_height = v; break;
        default: break;
    }
}

void api_impl::system__set_state_string (runtime::system_string_state z, const char* v) {
    switch (z){
        case runtime::system_string_state::title: engine_tasks.change_window_title = std::string (v); break;
        default: break;
    }
}

runtime::extension* api_impl::extension_get  (size_t id) const {
    assert (engine_extensions.find(id) != engine_extensions.end());
    runtime::extension* ext = engine_extensions.at(id).get();
    assert (ext);
    return ext;
};


void api_impl::tty_log (runtime::log_level level, const wchar_t* channel, const wchar_t* message)  const {

    engine_tasks.new_logs.emplace_back (log{ guid::random (), std::chrono::high_resolution_clock::now (), level, channel, message });
}



//--------------------------------------------------------------------------------------------------------------------//

void engine::process_user_log (const log& z_log) {

    const char* level;
    switch (z_log.level) {
        case runtime::log_level::debug: level = "DEBUG"; break;
        case runtime::log_level::info: level = "INFO"; break;
        case runtime::log_level::warning: level = "WARN"; break;
        case runtime::log_level::error: level = "ERROR"; break;
    };

#if TARGET_WIN32
    OutputDebugString ("[");
    OutputDebugString (level);
    OutputDebugString ("][");
    OutputDebugStringW (z_log.channel.c_str ());
    OutputDebugString ("] ");
    OutputDebugStringW (z_log.message.c_str());
    OutputDebugString ("\n");
#else
    printf ("%lld [%s][%ls] %ls \n", z_log.timestamp.time_since_epoch ().count (), level, z_log.channel.c_str (), z_log.message.c_str ());
#endif

    // todo: probably want to persist this lot somewhere and also provide access to it.
}

void engine::internal_update (sge::app::response& user_response, struct engine_state& engine_state, struct  engine_tasks& engine_tasks) {
    const auto tStart = std::chrono::high_resolution_clock::now ();

    // USER TASKS
    {
        if (engine_tasks.change_imgui_enabled.has_value ()) {
            engine_state.graphics.state.imgui_on = engine_tasks.change_imgui_enabled.value ();
            engine_tasks.change_imgui_enabled.reset ();
        }

        if (engine_tasks.change_fullscreen_enabled.has_value () && engine_state.host.set_window_fullscreen_fn.has_value ()) {
            const bool v = engine_tasks.change_fullscreen_enabled.value ();
            engine_state.host.set_window_fullscreen_fn.value() (v);
            engine_state.host.is_fullscreen = v;
            engine_tasks.change_fullscreen_enabled.reset ();
            engine_state.host.container_just_changed = true;
        }

        if (engine_tasks.change_window_title.has_value () && engine_state.host.set_window_title_fn.has_value ()) {
            const std::string& v = engine_tasks.change_window_title.value ();
            engine_state.host.set_window_title_fn.value () (v.c_str ());
            engine_state.host.window_title = v;
            engine_tasks.change_window_title.reset ();
        }

        if ((engine_tasks.change_canvas_width.has_value () || engine_tasks.change_canvas_height.has_value ()) && engine_state.host.set_window_size_fn.has_value ()) {
            const int vw = engine_tasks.change_canvas_width.has_value () ? engine_tasks.change_canvas_width.value () : engine_state.client.container_width;
            const int vh = engine_tasks.change_canvas_height.has_value() ? engine_tasks.change_canvas_height.value () : engine_state.client.container_height;

            const int adjusted_size_x = vw;
            const int adjusted_size_y = engine_state.graphics.state.imgui_on ? vh + imgui::ext::guess_main_menu_bar_height () : vh;

            engine_state.host.set_window_size_fn.value () (adjusted_size_x, adjusted_size_y);
            engine_tasks.change_canvas_width.reset ();
            engine_tasks.change_canvas_height.reset ();
            engine_state.host.container_just_changed = true;
        }

        if (engine_tasks.shutdown_request.has_value ()) {
            engine_state.host.shutdown_request_fn.value() ();
            engine_tasks.shutdown_request.reset ();
        }

        {
            for (int i = 0; i < engine_tasks.new_logs.size (); ++i) {
                process_user_log (engine_tasks.new_logs[i]);
            }
            engine_tasks.new_logs.clear ();
        }

    }

    // IMGUI
    {
        ImGuiIO& io = ImGui::GetIO ();
        const auto& input = engine_state.input;
        static int mouse_wheel_last_frame = 0;

        const int mouse_wheel_this_frame = (input.find (input_control_identifier::md_scrollwheel) != input.end ())
               ? std::get<input_digital_control> (input.at (input_control_identifier::md_scrollwheel))
               : 0;
        const int mouse_wheel_delta = mouse_wheel_this_frame - mouse_wheel_last_frame;

        mouse_wheel_last_frame = mouse_wheel_this_frame;

        auto p = (input.find (input_control_identifier::mp_position) != input.end ())
            ? std::get<input_point_control> (input.at (input_control_identifier::mp_position))
        : sge::math::point2 { 0, 0 };

        io.MousePos = ImVec2 (p.x, p.y);
        io.MouseDown[0] = input.find (input_control_identifier::mb_left) != input.end () && std::get<input_binary_control>(input.at(input_control_identifier::mb_left));
        io.MouseDown[2] = input.find (input_control_identifier::mb_middle) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::mb_middle));
        io.MouseDown[1] = input.find (input_control_identifier::mb_right) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::mb_right));
        io.DeltaTime = engine_state.instrumentation.frameTimer;
        io.MouseWheel = (float) mouse_wheel_delta;

        io.KeyCtrl = input.find (input_control_identifier::kb_control) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::kb_control));
        io.KeyShift = input.find (input_control_identifier::kb_shift) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::kb_shift));
        io.KeyAlt = input.find (input_control_identifier::kb_alt) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::kb_alt));
        io.KeySuper = input.find (input_control_identifier::kb_cmd) != input.end () && std::get<input_binary_control> (input.at (input_control_identifier::kb_cmd));

    }

    // VULKAN
    engine_state.graphics.update (
        user_response.push_constants_changed,
        user_response.uniform_changes,
        user_response.blob_changes,
        engine_state.instrumentation.frameTimer);

    // INSTRUMENTATION
    {
        engine_state.instrumentation.frameCounter++;
        const auto tEnd = std::chrono::high_resolution_clock::now ();
        const auto tDiff = std::chrono::duration<double, std::milli> (tEnd - tStart).count ();
        engine_state.instrumentation.frameTimer = (float)tDiff / 1000.0f;
        engine_state.instrumentation.totalTimer += engine_state.instrumentation.frameTimer;
        const float fpsTimer = (float)(std::chrono::duration<double, std::milli> (tEnd - engine_state.instrumentation.lastTimestamp).count ());
        if (fpsTimer > 1000.0f) {
            engine_state.instrumentation.lastFPS = static_cast<uint32_t>((float) engine_state.instrumentation.frameCounter * (1000.0f / fpsTimer));

            engine_state.instrumentation.frameCounter = 0;
            engine_state.instrumentation.lastTimestamp = tEnd;
        }
    }
}



engine::engine () {

    app::initialise ();
}

engine::~engine () {
    app::terminate ();
}

void engine::setup (
#if TARGET_WIN32
    HINSTANCE z_hinst,
    HWND z_hwnd
#elif TARGET_MACOSX
    void* z_view
#elif TARGET_LINUX
    xcb_connection_t* z_connection,
    xcb_window_t z_window
#else
#error
#endif
) {
    std::cout <<
        "\n"
        "   _________ ___________________\n"
        "  /   _____//  _____/\\_   _____/\n"
        "  \\_____  \\/   \\  ___ |    __)_\n"
        "  /        \\    \\_\\  \\|        \\\n"
        " /_______  /\\______  /_______  /\n"
        "         \\/        \\/  0.1.0 \\/\n"
        "\n\n";

    engine_state = std::make_unique<struct engine_state> ();

    std::stringstream ss;
    ss << SGE_VERSION_MAJOR (SGE_VERSION) << "." << SGE_VERSION_MINOR (SGE_VERSION) << "." << SGE_VERSION_PATCH (SGE_VERSION);
    engine_state->version = ss.str();

    engine_tasks = std::make_unique<struct engine_tasks> ();

    auto configuration = sge::app::get_configuration ();

    engine_tasks->change_window_title = configuration.app_name;

#if TARGET_WIN32
    engine_state->platform.hinst = z_hinst;
    engine_state->platform.hwnd = z_hwnd;
    engine_state->graphics.create (z_hinst, z_hwnd, engine_state->client.container_width, engine_state->client.container_height);
#elif TARGET_MACOSX
    engine_state->platform.view = z_view;
    engine_state->graphics.create (z_view, engine_state->client.container_width, engine_state->client.container_height);
#elif TARGET_LINUX
    engine_state->graphics.create (z_connection, z_window, engine_state->client.container_width, engine_state->client.container_height);
#else
#error
#endif

    engine_api = std::make_unique<api_impl> (*engine_state, *engine_tasks, engine_extensions);

    auto& standard_extensions = sge::app::internal::get_standard_extensions ();

    // I am certain that this can be done in a much better way with some template wizardary.
    for (int i = 0; i < standard_extensions.views.size(); ++i) {
        size_t id = standard_extensions.views[i].first;
        auto& new_fn = standard_extensions.views[i].second;
        runtime::view* view = new_fn (*engine_api);
        engine_extensions[id] = std::unique_ptr<runtime::extension> (view);
    }

    for (int i = 0; i < standard_extensions.systems.size(); ++i) {
        size_t id = standard_extensions.systems[i].first;
        auto& new_fn = standard_extensions.systems[i].second;
        runtime::system* system = new_fn (*engine_api);
        engine_extensions[id] = std::unique_ptr<runtime::extension> (system);
    }

    auto& user_extensions = sge::app::get_extensions ();

    for (int i = 0; i < user_extensions.views.size(); ++i) {
        size_t id = user_extensions.views[i].first;
        auto& new_fn = user_extensions.views[i].second;
        runtime::view* view = new_fn (*engine_api);
        engine_extensions[id] = std::unique_ptr<runtime::extension> (view);
    }

    for (int i = 0; i < user_extensions.systems.size(); ++i) {
        size_t id = user_extensions.systems[i].first;
        auto& new_fn = user_extensions.systems[i].second;
        runtime::system* system = new_fn (*engine_api);
        engine_extensions[id] = std::unique_ptr<runtime::extension> (system);
    }

    user_response = std::make_unique<struct app::response> (app::get_content ().uniforms.size (), app::get_content ().blobs.size ());
    user_api = app::internal::create_user_api (*engine_api);

}

void engine::start () {

    sge::app::start (*user_api);
    engine_state->graphics.create_systems (std::bind(&engine::imgui, this));
}

void engine::update (client_state& z_container, input_state& z_input) {

    engine_state->host.container_just_changed = false;

    /*
    if (z_input.find(input_control_identifier::kq_caps_lk) != z_input.end()) {
        bool locked = std::get<input_quaternary_control> (z_input.at (input_control_identifier::kq_caps_lk)).first;
        bool pressed = std::get<input_quaternary_control> (z_input.at (input_control_identifier::kq_caps_lk)).second;
        std::cout << "caps lk (" << locked << ", " << pressed << ")" << '\n';
    }*/

    if (z_container.container_width != engine_state->client.container_width
        || z_container.container_height != engine_state->client.container_height
        || z_container.is_resizing) {
        engine_state->host.container_just_changed = true;
    }

    // copy new state provided by the host
    engine_state->client = z_container;
    engine_state->input = z_input;

    internal_update (*user_response, *engine_state, *engine_tasks);

    // update all registered extensions
    for (auto& kvp : engine_extensions) {
        if (kvp.second->is_active())
            kvp.second->invoke_update ();
    }

    // update the user's app
    sge::app::update (*user_response, *user_api);

}

void engine::stop () {
    sge::app::stop (*user_api);
}

void engine::shutdown () {
    app::internal::delete_user_api (user_api);
    user_response.reset ();
    engine_extensions.clear ();
    engine_state->graphics.destroy ();
    engine_tasks.reset ();
    engine_state.reset ();
}















//====================================================================================================================//

void engine::imgui () {

    static bool show_about_window = false;
    static bool show_engine_host_window = false;
    static bool show_engine_graphics_window = false;
    static bool show_engine_memory_window = false;
    static bool show_dear_imgui_demo_window = false;

    // top level imgui fn, all imgui calls are from this call.
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("SGE")) {
            if (ImGui::MenuItem("About", NULL, show_about_window)) {
                show_about_window = !show_about_window;
            }
            ImGui::Separator();
#if TARGET_WIN32
            if (ImGui::MenuItem("Quit", "Alt+F4")) { /* todo */ }
#elif TARGET_MACOSX
            if (ImGui::MenuItem("Quit", "CMD+Q")) { /* todo */ }
#endif
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Engine")) {

            if (ImGui::MenuItem("Host", NULL, show_engine_host_window)) {
                show_engine_host_window = !show_engine_host_window;
            }

            if (ImGui::MenuItem("Graphics", NULL, show_engine_graphics_window)) {
                show_engine_graphics_window = !show_engine_graphics_window;
            }

            if (ImGui::MenuItem("Memory", NULL, show_engine_memory_window)) {
                show_engine_memory_window = !show_engine_memory_window;
            }


            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Runtime")) {
            for (auto& kvp : engine_extensions) {
                const size_t id = kvp.first;
                engine_extensions[id]->invoke_debug_menu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("User")) {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Dear ImGui Demo", NULL, show_dear_imgui_demo_window)) {
                show_dear_imgui_demo_window = !show_dear_imgui_demo_window;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    for (auto& kvp : engine_extensions) {
        const size_t id = kvp.first;
        engine_extensions[id]->invoke_debug_ui();
    }

    if (show_about_window)           about_window    (&show_about_window);
    if (show_engine_host_window)     host_window     (&show_engine_host_window);
    if (show_engine_graphics_window) graphics_window (&show_engine_graphics_window);
    if (show_engine_memory_window)   memory_window   (&show_engine_memory_window);

    if (show_dear_imgui_demo_window) ImGui::ShowDemoWindow();

    sge::app::debug_ui (*user_response, *user_api);
}


void engine::about_window (bool* show) {
    const int w  = 240;
    ImGui::SetNextWindowSize(ImVec2 (w, 260), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2 ((engine_state->client.container_width / 2.0f) - (w/2.0f), 130), ImGuiCond_Once);
    ImGui::Begin("About", show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    static bool first_run = true;
    static std::vector<data::vertex_pos_col> toy_obj_verts;
    static std::vector<uint32_t> toy_obj_indices;
    static float toy_cam_zn = -0.1f, toy_cam_zf = -300.0f, toy_cam_fov = 45.0f;
    static math::vector3 toy_cam_pos = math::vector3 { 0, 0, 5 };
    static math::quaternion toy_cam_orientation = math::quaternion::identity;
    static math::rect toy_container { { 0, 0 }, { w - 15, 140 }};
    static math::vector3 toy_obj_pos = math::vector3 { 0, 0.0f, -1 };
    static math::quaternion toy_obj_orientation = math::quaternion::identity;
    static float toy_obj_t = 0.0f, toy_obj_speed = 0.8f;

    if (first_run) {
        //std::vector<data::vertex_pos_norm> verts;
        //data::get_teapot (verts, toy_obj_indices, 1.0f, 4);
        //for (auto& v : verts)
        //    toy_obj_verts.emplace_back(data::vertex_pos_col {v.position, 0xFFFFFFFF} );
        data::get_colourful_cube (toy_obj_verts, toy_obj_indices);
        first_run = false;
    }

    toy_obj_t += engine_state->instrumentation.frameTimer * toy_obj_speed;
    toy_obj_orientation = math::quaternion().set_from_yaw_pitch_roll(toy_obj_t, 2.0 * toy_obj_t, -toy_obj_t);

    imgui::ext::draw_user_triangles (
        toy_container,
        true,
        toy_cam_pos,
        toy_cam_orientation,
        toy_cam_fov * math::DEG2RAD,
        toy_cam_zn,
        toy_cam_zf,
        toy_obj_verts,
        toy_obj_indices,
        toy_obj_pos,
        toy_obj_orientation,
        0x00000000);

    ImGui::Text ("SGE ~ Simple Graphics Engine");
    ImGui::Text ("      written by A.J.Pook");
    ImGui::Text ("      copyright © 2020");
    ImGui::Text ("      license: MIT");

    ImGui::End();
}

void engine::host_window (bool* show) {
    ImGui::SetNextWindowPos(ImVec2 (100, 130), ImGuiCond_Once);
    ImGui::Begin("SGE Host", show, ImGuiWindowFlags_NoCollapse);

    const int display_width = engine_state->client.max_container_width;
    const int display_height = engine_state->client.max_container_height;

    const int window_width = engine_state->client.window_width;
    const int window_height = engine_state->client.window_height;
    const int window_x = engine_state->client.window_position_x;
    const int window_y = engine_state->client.window_position_y;

    const int container_width = engine_state->client.container_width;
    const int container_height = engine_state->client.container_height;
    const int container_x = engine_state->client.container_position_x;
    const int container_y = engine_state->client.container_position_y;

    const int canvas_width = engine_state->graphics.get_user_viewport_width ();
    const int canvas_height = engine_state->graphics.get_user_viewport_height ();
    const int canvas_x = engine_state->graphics.get_user_viewport_x ();
    const int canvas_y = engine_state->graphics.get_user_viewport_y ();

    ImGui::Text ("Display size: %dx%d", display_width, display_height);
    ImGui::Text ("Window size: %dx%d", window_width, window_height);
    ImGui::Text ("Window position: %dx%d", window_x, window_y);
    ImGui::Text ("Container size: %dx%d", container_width, container_height);
    ImGui::Text ("Container position: %dx%d", container_x, container_y);
    ImGui::Text ("Canvas size: %dx%d", canvas_width, canvas_height);
    ImGui::Text ("Canvas position: %dx%d", canvas_x, canvas_y);

    ImGui::Dummy (ImVec2 (0, 10));

    {
        const float im_content_w = ImGui::GetWindowContentRegionWidth ();
        auto dl = ImGui::GetWindowDrawList ();
        const ImVec2 im_cp = ImGui::GetCursorScreenPos ();
        const float im_scale = im_content_w / (float) display_width;

        const auto im_display_size = ImVec2 (im_scale * display_width, im_scale * display_height);
        const auto im_display_min = im_cp;
        const auto im_display_max = ImVec2 (im_display_min.x + im_display_size.x, im_display_min.y + im_display_size.y);
        dl->AddRectFilled (im_display_min, im_display_max, 0xFFA18A7C);

        const auto im_window_size = ImVec2 (im_scale * window_width, im_scale * window_height);
        const auto im_window_min = ImVec2 (im_display_min.x + (im_scale * window_x), im_display_min.y + (im_scale * window_y));
        const auto im_window_max = ImVec2 (im_window_min.x + im_window_size.x, im_window_min.y + im_window_size.y);
        dl->AddRectFilled (im_window_min, im_window_max, 0xFF00FFDD);

        const auto im_container_size = ImVec2 (im_scale * container_width, im_scale * container_height);
        const auto im_container_min = ImVec2 (im_window_min.x + (im_scale * container_x), im_window_min.y + (im_scale * container_y));
        const auto im_container_max = ImVec2 (im_container_min.x + im_container_size.x, im_container_min.y + im_container_size.y);
        dl->AddRectFilled (im_container_min, im_container_max, 0xFF0099FF);

        const auto im_canvas_size = ImVec2 (im_scale * canvas_width, im_scale * canvas_height);
        const auto im_canvas_min = ImVec2 (im_container_min.x + (im_scale * canvas_x), im_container_min.y + (im_scale * canvas_y));
        const auto im_canvas_max = ImVec2 (im_canvas_min.x + im_canvas_size.x, im_canvas_min.y + im_canvas_size.y);
        dl->AddRectFilled (im_canvas_min, im_canvas_max, 0xFFFFC9FB);

        ImGui::SetCursorScreenPos (ImVec2 (im_cp.x, im_cp.y + im_display_size.y));

    }
    ImGui::End ();

}

void engine::graphics_window (bool* show) {
    ImGui::SetNextWindowPos(ImVec2 (100, 130), ImGuiCond_Once);

    ImGui::Begin("SGE Graphics", show, ImGuiWindowFlags_NoCollapse);

    ImGui::Text ("Compute target size: %dx%d",
        engine_state->graphics.compute_target->current_width (),
        engine_state->graphics.compute_target->current_height ());


    ImGui::End ();
}

void engine::memory_window (bool* show) {
    ImGui::SetNextWindowPos(ImVec2 (100, 130), ImGuiCond_Once);
    ImGui::Begin("SGE Memory", show, ImGuiWindowFlags_NoCollapse);

    //engine_state->graphics.kernel->custom_allocator->debug_ui_content();
    ImGui::End ();
}

}

