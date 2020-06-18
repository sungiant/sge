// SGE-OSX
// Reference SGE host implementation.
// ---------------------------------- //

#if TARGET_MACOSX

#include <pthread.h>

#include <chrono>
#include <iomanip>
#include <unordered_set>
#include <algorithm>
#include <optional>
#include <vector>
#include <queue>
#include <functional>
#include <chrono>
#include <string>
#include <thread>

#include <Cocoa/Cocoa.h>
#include <MetalKit/MTKView.h>
#include <IOKit/hid/IOHIDManager.h>

#include "sge_core.hh"
#include "sge_app.hh"

#define SGE_OSX_INPUT_DEBUG 0


// IOKit Gamepad (stand alone class - independent of SGE)
// -------------------------------------------------------------------------- //
// Tested against:
// * N30 Pro
// * Wired PS4 DualShock
// -------------------------------------------------------------------------- //
class iokit_gamepad {

public:
    
    enum class index { one, two, three, four };
    struct vector { float x; float y; };
    enum class button {
        dpad_up, dpad_down, dpad_left, dpad_right,
        action_south, action_east, action_west, action_north,
        option_left, option_middle, option_right,
        left_thumb, right_thumb, left_shoulder, right_shoulder
    };
    
    vector                  get_left_stick      ()                  const { return get_left_stick (default_connection ().value_or(index::one)).value_or (vector { 0.0f, 0.0f }); }
    vector                  get_right_stick     ()                  const { return get_right_stick (default_connection ().value_or(index::one)).value_or (vector { 0.0f, 0.0f }); }
    float                   get_left_trigger    ()                  const { return get_left_trigger (default_connection ().value_or(index::one)).value_or (0.0f); }
    float                   get_right_trigger   ()                  const { return get_right_trigger (default_connection ().value_or(index::one)).value_or (0.0f);}
    bool                    is_button_pressed   (button z)          const { return is_button_pressed (default_connection ().value_or(index::one), z).value_or (false);}

    std::optional<vector>   get_left_stick      (index i)           const { int idx = convert(i); if (connections[idx].has_value()) { return connections[idx].value().second.left_stick; } return std::nullopt; }
    std::optional<vector>   get_right_stick     (index i)           const { int idx = convert(i); if (connections[idx].has_value()) { return connections[idx].value().second.right_stick; } return std::nullopt; }
    std::optional<float>    get_left_trigger    (index i)           const { int idx = convert(i); if (connections[idx].has_value()) { return connections[idx].value().second.left_trigger; } return std::nullopt; }
    std::optional<float>    get_right_trigger   (index i)           const { int idx = convert(i); if (connections[idx].has_value()) { return connections[idx].value().second.right_trigger; } return std::nullopt; }
    std::optional<bool>     is_button_pressed   (index i, button z) const { int idx = convert(i); if (connections[idx].has_value()) { return connections[idx].value().second.pressed_buttons.find(z) != connections[idx].value().second.pressed_buttons.end(); } return std::nullopt; }
    
                            iokit_gamepad       ()                        { start (); }
                            ~iokit_gamepad      ()                        { stop (); }
    void                    update              ()                        { process (); }
    std::optional<index>    default_connection  ()                  const { for (int i = 0; i < CONNECTION_LIMIT; ++i) { if (connections[i].has_value()) return (index) i; } return std::nullopt; }

private:
    static constexpr int CONNECTION_LIMIT = 4;
    
    enum Cookie {
        BUTTON_ACTION_SOUTH = 2, BUTTON_ACTION_EAST = 3, BUTTON_ACTION_WEST = 4, BUTTON_ACTION_NORTH = 5,
        BUTTON_LEFT_SHOULDER = 6, BUTTON_RIGHT_SHOULDER = 7, BUTTON_LEFT_TRIGGER = 8, BUTTON_RIGHT_TRIGGER = 9,
        BUTTON_OPTION_LEFT = 10, BUTTON_OPTION_RIGHT = 11, BUTTON_LEFT_THUMB = 12, BUTTON_RIGHT_THUMB = 13,
        BUTTON_OPTION_EXTRA = 14, BUTTON_OPTION_MIDDLE = 15,
        AXIS_LEFT_X = 16, AXIS_LEFT_Y = 17, AXIS_RIGHT_X = 18, AXIS_RIGHT_Y = 19,
        AXIS_DPAD = 20, AXIS_LEFT_TRIGGER = 22, AXIS_RIGHT_TRIGGER = 23,
    };
    
    struct attached_event   { IOHIDDeviceRef device; };
    struct detached_event   { IOHIDDeviceRef device; };
    struct input_event      { IOHIDDeviceRef device; IOHIDElementCookie identifier; IOHIDElementType type; int value; };

    struct device_state {
        std::unordered_set<button>  pressed_buttons;
        vector                      left_stick;
        vector                      right_stick;
        float                       left_trigger;
        float                       right_trigger;
    };
    
    struct input_reference {
        input_reference (iokit_gamepad& zp, IOHIDDeviceRef zd)
            : parent (zp), device (zd)
        {}
        iokit_gamepad&              parent;
        const IOHIDDeviceRef        device;
    };
    
    typedef std::variant<attached_event, detached_event, input_event>       event;
    typedef std::chrono::high_resolution_clock::time_point                  timestamp;
    typedef std::optional<std::pair<IOHIDDeviceRef, device_state>>          connection;
    
    // accessed on hid thread
    std::queue<event>                                                       event_queue;
    std::unordered_map<IOHIDDeviceRef, std::unique_ptr<input_reference>>    input_callback_data; // crap needed by the input callback
    
    // local only
    IOHIDManagerRef                                                         hid_manager = nullptr;
    CFRunLoopRef                                                            event_thread_loop = nullptr;
    pthread_t                                                               event_thread;
    pthread_mutex_t                                                         mutex;
    timestamp                                                               last_gamepad_update;
    timestamp                                                               last_gamepad_scan;
    std::array <connection, CONNECTION_LIMIT>                               connections;

    
    //
    // Implementation
    // --------------

    void start () {
        pthread_mutex_init (&mutex, nullptr);
        assert (pthread_create (&event_thread, nullptr, thread_start, this) == 0);
        while (event_thread_loop == nullptr)
            { ; }
        hid_manager = IOHIDManagerCreate (kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        assert (hid_manager);
        {
            constexpr int page_desktop = kHIDPage_GenericDesktop;
            constexpr int usage_joystick = kHIDUsage_GD_Joystick;
            constexpr int usage_gamepad = kHIDUsage_GD_GamePad;
            constexpr int usage_controller = kHIDUsage_GD_MultiAxisController;
            CFStringRef keys[] { CFSTR (kIOHIDDeviceUsagePageKey), CFSTR (kIOHIDDeviceUsageKey) };
            const auto desktop = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &page_desktop);
            const auto joystick = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &usage_joystick);
            const auto gamepad = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &usage_gamepad);
            const auto controller = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &usage_controller);
            const std::vector<CFNumberRef> desktop_joystick { desktop, joystick };
            const std::vector<CFNumberRef> desktop_gamepad { desktop, gamepad };
            const std::vector<CFNumberRef> desktop_controller { desktop, controller };
            const std::vector<CFDictionaryRef> devices {
                CFDictionaryCreate (kCFAllocatorDefault, (const void **)keys, (const void **)desktop_joystick.data(), desktop_joystick.size(), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks),
                CFDictionaryCreate (kCFAllocatorDefault, (const void **)keys, (const void **)desktop_gamepad.data(), desktop_gamepad.size(), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks),
                CFDictionaryCreate (kCFAllocatorDefault, (const void **)keys, (const void **)desktop_controller.data(), desktop_controller.size(), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
            CFRelease (controller); CFRelease (gamepad); CFRelease (joystick); CFRelease (desktop);
            const CFArrayRef multiple = CFArrayCreate (kCFAllocatorDefault, (const void **) devices.data(), devices.size(), &kCFTypeArrayCallBacks);
            std::for_each (devices.begin(), devices.end(), [](CFDictionaryRef r){ CFRelease (r); });
            IOHIDManagerSetDeviceMatchingMultiple (hid_manager, multiple);
            CFRelease (multiple);
        }
        IOHIDManagerRegisterDeviceMatchingCallback (hid_manager, iohid_attached_callback, this);
        IOHIDManagerRegisterDeviceRemovalCallback (hid_manager, iohid_detached_callback, this);
        IOReturn r = IOHIDManagerOpen (hid_manager, kIOHIDOptionsTypeNone);
        if (r != kIOReturnSuccess) {
#if SGE_OSX_INPUT_DEBUG
            std::cout << "Unexpected return code [" << r << "] from IOHIDManagerOpen (" << hid_manager << ")" << '\n';
#endif
            assert (false);
        }
        IOHIDManagerScheduleWithRunLoop (hid_manager, CFRunLoopGetCurrent (), CFSTR ("RunLoopModeDiscovery"));
    }
    
    void stop () {
        for (int i = 0; i < CONNECTION_LIMIT; ++i) {
            if (connections[i].has_value())
                remove_connection(connections[i].value().first);
        }
        pthread_mutex_lock (&mutex);
        while (event_queue.size())
            event_queue.pop();
        input_callback_data.clear();
        pthread_mutex_unlock (&mutex);
        if (event_thread_loop != nullptr) {
          pthread_cancel (event_thread);
          event_thread_loop = nullptr;
        }
        pthread_mutex_destroy (&mutex);
        
        IOHIDManagerUnscheduleFromRunLoop (hid_manager, CFRunLoopGetCurrent (), kCFRunLoopCommonModes);

        IOReturn r = IOHIDManagerClose (hid_manager, kIOHIDOptionsTypeNone);
        if (r != kIOReturnSuccess) {
#if SGE_OSX_INPUT_DEBUG
            std::cout << "Unexpected return code [" << r << "] from IOHIDManagerClose (" << hid_manager << ")" << '\n';
#endif
            assert (false);
        }
        CFRelease (hid_manager);
        hid_manager = nullptr;
    }
    
    void process () {
        const auto now = std::chrono::high_resolution_clock::now ();
        if (now - last_gamepad_scan >= std::chrono::seconds (1)) { // scan
            CFRunLoopRunInMode (CFSTR ("RunLoopModeDiscovery"), 0, true);
            last_gamepad_scan = now;
        }
        if (now - last_gamepad_update >= std::chrono::milliseconds (5)) { // update
            pthread_mutex_lock (&mutex);
            while (event_queue.size ()) {
                event event = event_queue.front ();
                event_queue.pop ();
                if (attached_event* attached = std::get_if<attached_event> (&event)) { add_connection (attached->device); }
                if (detached_event* detached = std::get_if<detached_event> (&event)) { remove_connection (detached->device); }
                if (input_event* input = std::get_if<input_event> (&event)) { handle_input_event (input->device, input->identifier, input->value); }
            }
            pthread_mutex_unlock (&mutex);
            last_gamepad_update = now;
        }
    }
    
    void add_connection (IOHIDDeviceRef device) {
        auto idx = get_next_connection_index ();
        if (idx.has_value ()) {
#if SGE_OSX_INPUT_DEBUG
            std::cout << "Adding device [" << idx.value () << "] (" << device << ")." << '\n';
#endif
            connections[idx.value ()] = { device, device_state {} };
        }
        else {
#if SGE_OSX_INPUT_DEBUG
            std::cout << "Ignoring device: " << device << ", connection limit exceeded." << '\n';
#endif
        }
    }
    
    void remove_connection (IOHIDDeviceRef device) {
        const auto idx = get_connection_index (device);
        if (idx.has_value ()) {
#if SGE_OSX_INPUT_DEBUG
            std::cout << "Removing device [" << idx.value () << "] (" << device << ")." << '\n';
#endif
            connections[idx.value ()].reset();
        }
    }
    void handle_input_event (IOHIDDeviceRef device, IOHIDElementCookie identifier, int value) {
        const auto idx = get_connection_index (device);
        assert (idx.has_value());
        const int index = idx.value();
        assert (connections[index].has_value());
        auto& state = connections[index].value ().second;
        switch (identifier) {
            // buttons
            #define CASE(x, y) { case x: { if (value > 0) state.pressed_buttons.insert (y); else state.pressed_buttons.erase (y); break; } }
            CASE (BUTTON_ACTION_SOUTH, button::action_south);
            CASE (BUTTON_ACTION_EAST, button::action_east);
            CASE (BUTTON_ACTION_WEST, button::action_west);
            CASE (BUTTON_ACTION_NORTH, button::action_north);
            CASE (BUTTON_LEFT_SHOULDER, button::left_shoulder);
            CASE (BUTTON_RIGHT_SHOULDER, button::right_shoulder);
            CASE (BUTTON_OPTION_LEFT, button::option_left);
            CASE (BUTTON_OPTION_MIDDLE, button::option_middle);
            CASE (BUTTON_OPTION_RIGHT, button::option_right);
            CASE (BUTTON_LEFT_THUMB, button::left_thumb);
            CASE (BUTTON_RIGHT_THUMB, button::right_thumb);
            #undef CASE
            // sticks
            case AXIS_LEFT_X: case AXIS_LEFT_Y: case AXIS_RIGHT_X: case AXIS_RIGHT_Y: {
                float v = (((float) value / 255.0f) * 2.0f) - 1.0f;
                const float THUMBSTICK_DEADZONE = 0.1f;
                if (v < THUMBSTICK_DEADZONE && v > -THUMBSTICK_DEADZONE) { v = 0.0f; }

                if (identifier == AXIS_LEFT_X)      state.left_stick.x = v;
                else if (identifier == AXIS_LEFT_Y) state.left_stick.y = -v;
                else if (identifier == AXIS_RIGHT_X) state.right_stick.x = v;
                else if (identifier == AXIS_RIGHT_Y) state.right_stick.y = -v;
                
                break;
            }
            // dpad
            case AXIS_DPAD: {
                #define FN(x) { state.pressed_buttons.insert (x); }
                switch (value) {
                    case 0: FN (button::dpad_up);                                   break;
                    case 1: FN (button::dpad_up);       FN (button::dpad_right);    break;
                    case 2: FN (button::dpad_right);                                break;
                    case 3: FN (button::dpad_right);    FN (button::dpad_down);     break;
                    case 4: FN (button::dpad_down);                                 break;
                    case 5: FN (button::dpad_left);     FN (button::dpad_down);     break;
                    case 6: FN (button::dpad_left);                                 break;
                    case 7: FN (button::dpad_left);     FN (button::dpad_up);       break;
                    case 8:
                        state.pressed_buttons.erase (button::dpad_up);
                        state.pressed_buttons.erase (button::dpad_right);
                        state.pressed_buttons.erase (button::dpad_down);
                        state.pressed_buttons.erase (button::dpad_left);            break;
                }
                #undef FN
                // triggers
                case AXIS_LEFT_TRIGGER: case AXIS_RIGHT_TRIGGER: {
                    float v = (float) value / 255.0f;
                    const float TRIGGER_DEADZONE = 0.01f;
                    if (value < TRIGGER_DEADZONE) { v = 0.0f; }
                    if (identifier == 22)      state.left_trigger = v;
                    else if (identifier == 23) state.right_trigger = v;
                    break;
                }
                // button triggers (after real triggers as both fire)
                case BUTTON_LEFT_TRIGGER: { if (value > 0) state.left_trigger = 1.0f; else state.left_trigger = 0.0f; break; }
                case BUTTON_RIGHT_TRIGGER: { if (value > 0) state.right_trigger = 1.0f; else state.right_trigger = 0.0f; break; }
            }
        }
    }
    
    //
    // Callbacks
    // --------------
    
    static void* thread_start (void* context) {
        iokit_gamepad* system = static_cast<iokit_gamepad*>(context);
        system->event_thread_loop = CFRunLoopGetCurrent ();
        while (true) {
            CFRunLoopRun ();
            std::this_thread::sleep_for (std::chrono::milliseconds (10));
        }
        return nullptr;
    }
    
    static void iohid_attached_callback (void* context, IOReturn result, void* sender, IOHIDDeviceRef device) {
        iokit_gamepad* gp = static_cast<iokit_gamepad*>(context);
        pthread_mutex_lock (&gp->mutex);
        gp->input_callback_data[device] = std::make_unique<input_reference> (*gp, device);
        gp->event_queue.push (attached_event { device });
        IOHIDDeviceRegisterInputValueCallback (device, iohid_input_callback, gp->input_callback_data[device].get());
        IOHIDDeviceScheduleWithRunLoop (device, gp->event_thread_loop, kCFRunLoopDefaultMode);
        pthread_mutex_unlock (&gp->mutex);
    }
    
    static void iohid_detached_callback (void* context, IOReturn result, void* sender, IOHIDDeviceRef device) {
        iokit_gamepad* gp = static_cast<iokit_gamepad*>(context);
        pthread_mutex_lock (&gp->mutex);
        gp->input_callback_data.erase (device);
        gp->event_queue.push (detached_event { device });
        pthread_mutex_unlock (&gp->mutex);
    }
    
    static void iohid_input_callback (void* context, IOReturn result, void* sender, IOHIDValueRef value) {
        static const std::unordered_set<int> interesting_cookies = {
            BUTTON_ACTION_SOUTH, BUTTON_ACTION_EAST, BUTTON_ACTION_WEST, BUTTON_ACTION_NORTH,
            BUTTON_LEFT_SHOULDER, BUTTON_RIGHT_SHOULDER, BUTTON_LEFT_TRIGGER, BUTTON_RIGHT_TRIGGER,
            BUTTON_OPTION_LEFT, BUTTON_OPTION_RIGHT, BUTTON_LEFT_THUMB, BUTTON_RIGHT_THUMB,
            BUTTON_OPTION_EXTRA, BUTTON_OPTION_MIDDLE,
            AXIS_LEFT_X, AXIS_LEFT_Y, AXIS_RIGHT_X, AXIS_RIGHT_Y,
            AXIS_DPAD, AXIS_LEFT_TRIGGER, AXIS_RIGHT_TRIGGER,
        };
        
        input_reference* ir = static_cast<input_reference*>(context);
        const IOHIDElementRef element = IOHIDValueGetElement (value);
        IOHIDElementType type = IOHIDElementGetType (element);
        const IOHIDElementCookie cookie = IOHIDElementGetCookie (element);
        const int int_value = IOHIDValueGetIntegerValue (value);
        
        pthread_mutex_lock (&ir->parent.mutex);
        if (interesting_cookies.find (cookie) != interesting_cookies.end()) {
            if (ir->parent.event_queue.size () < 512) {
                ir->parent.event_queue.push (input_event { ir->device, cookie, type, int_value });
            }
            else assert (false); // too many events!
        }
        pthread_mutex_unlock (&ir->parent.mutex);
    }
    
    //
    // Utils
    // --------------
    
    int convert (index z) const { switch (z) { case index::one: return 0; case index::two: return 1; case index::three: return 2; case index::four: return 3; } }
    
    std::optional<int> get_next_connection_index () const {
        for (int i = 0; i < CONNECTION_LIMIT; ++i) {
            if (!connections[i].has_value())
                return i;
        }
        return std::nullopt;
    }
    std::optional<int> get_connection_index (IOHIDDeviceRef device) const {
        for (int i = 0; i < CONNECTION_LIMIT; ++i) {
            if (connections[i].has_value() && connections[i].value().first == device)
                return i;
        }
        return std::nullopt;
    }
};



// PROGRAM
//################################################################################################//

std::unordered_set<wchar_t>             g_keyboard_pressed_characters;
std::unordered_set<int>                 g_keyboard_pressed_fns;
bool                                    g_mouse_left;
bool                                    g_mouse_middle;
bool                                    g_mouse_right;
float                                   g_mouse_position_x;
float                                   g_mouse_position_y;
int                                     g_mouse_scrollwheel;
iokit_gamepad                           g_gamepad;
bool                                    g_fullscreen;
bool                                    g_is_resizing;
int                                     g_container_width;
int                                     g_container_height;
std::optional<sge::math::point2>        g_target_window_size;
std::unique_ptr<sge::core::engine>      g_sge;

//################################################################################################//

void calculate_sge_container_state (sge::core::container_state& container) {
    container.is_resizing = g_is_resizing;
    container.current_width = g_container_width;
    container.current_height = g_container_height;
}

void calculate_sge_input_state (sge::input_state& input) {
    // keyboard
    int keyboard_i = 0;
    assert (g_keyboard_pressed_characters.size () <= 10);
    for (wchar_t character : g_keyboard_pressed_characters) {
        int i = static_cast<int> (sge::input_control_identifier::kc_0) + keyboard_i;
        auto identifier = static_cast<sge::input_control_identifier> (i);
        input[identifier] = static_cast<wchar_t> (character);
        ++keyboard_i;
    }
    
    
    #define FN(x, y) { if (g_keyboard_pressed_fns.find (x) != g_keyboard_pressed_fns.end()) input[sge::input_control_identifier::kb_ ## y] = true; }
    FN (27, escape); FN (13, enter); FN (32, spacebar);
    FN (NSEventModifierFlagShift, shift); FN (NSEventModifierFlagControl, control); FN (NSEventModifierFlagOption, alt);
    FN (127, backspace); FN (9, tab);
    FN (NSInsertFunctionKey, ins); FN (NSDeleteFunctionKey, del);
    FN (NSHomeFunctionKey, home); FN (NSEndFunctionKey, end);
    FN (NSPageUpFunctionKey, page_up); FN (NSPageDownFunctionKey, page_down);
    FN (NSMenuFunctionKey, right_click); FN (NSPrintScreenFunctionKey, prt_sc); FN (NSPauseFunctionKey, pause);
    FN (NSUpArrowFunctionKey, up); FN (NSDownArrowFunctionKey, down);
    FN (NSLeftArrowFunctionKey, left); FN (NSRightArrowFunctionKey, right);
    FN (NSEventModifierFlagCommand, cmd); FN (3, numpad_enter);
    FN (NSF1FunctionKey, f1); FN (NSF2FunctionKey, f2); FN (NSF3FunctionKey, f3);
    FN (NSF4FunctionKey, f4); FN (NSF5FunctionKey, f5); FN (NSF6FunctionKey, f6);
    FN (NSF7FunctionKey, f7); FN (NSF8FunctionKey, f8); FN (NSF9FunctionKey, f9);
    FN (NSF10FunctionKey, f10); FN (NSF11FunctionKey, f11); FN (NSF12FunctionKey, f12);
    #undef FN
    
    for (wchar_t character : g_keyboard_pressed_characters) {
        switch (toupper (character)) {
            #define CASE(x, y) { case x: input[sge::input_control_identifier::kb_ ## y] = true; break; }
            CASE ('A', a); CASE ('B', b); CASE ('C', c); CASE ('D', d); CASE ('E', e);
            CASE ('F', f); CASE ('G', g); CASE ('H', h); CASE ('I', i); CASE ('J', j);
            CASE ('K', k); CASE ('L', l); CASE ('M', m); CASE ('N', n); CASE ('O', o);
            CASE ('P', p); CASE ('Q', q); CASE ('R', r); CASE ('S', s); CASE ('T', t);
            CASE ('U', u); CASE ('V', v); CASE ('W', w); CASE ('X', x); CASE ('Y', y);
            CASE ('Z', z); CASE ('0', 0); CASE ('1', 1); CASE ('2', 2); CASE ('3', 3);
            CASE ('4', 4); CASE ('5', 5); CASE ('6', 6); CASE ('7', 7); CASE ('8', 8);
            CASE ('9', 9); CASE ('+', plus); CASE ('-', minus); CASE (',', comma); CASE ('.', period);
            #undef CASE
            default: break;
        }
    }
    
    for (wchar_t character : g_keyboard_pressed_characters) { // not sure how to get this to work
        if (g_keyboard_pressed_fns.find (NSEventModifierFlagNumericPad) != g_keyboard_pressed_fns.end()) {
            if (character == '0') { input[sge::input_control_identifier::kb_numpad_0]; continue; }
            if (character == '1') { input[sge::input_control_identifier::kb_numpad_1]; continue; }
            if (character == '2') { input[sge::input_control_identifier::kb_numpad_2]; continue; }
            if (character == '3') { input[sge::input_control_identifier::kb_numpad_3]; continue; }
            if (character == '4') { input[sge::input_control_identifier::kb_numpad_4]; continue; }
            if (character == '5') { input[sge::input_control_identifier::kb_numpad_5]; continue; }
            if (character == '6') { input[sge::input_control_identifier::kb_numpad_6]; continue; }
            if (character == '7') { input[sge::input_control_identifier::kb_numpad_7]; continue; }
            if (character == '8') { input[sge::input_control_identifier::kb_numpad_8]; continue; }
            if (character == '9') { input[sge::input_control_identifier::kb_numpad_9]; continue; }
            if (character == '.') { input[sge::input_control_identifier::kb_numpad_decimal]; continue; }
            if (character == '/') { input[sge::input_control_identifier::kb_numpad_divide]; continue; }
            if (character == '*') { input[sge::input_control_identifier::kb_numpad_multiply]; continue; }
            if (character == '-') { input[sge::input_control_identifier::kb_numpad_subtract]; continue; }
            if (character == '+') { input[sge::input_control_identifier::kb_numpad_add]; continue; }
            if (character == '=') { input[sge::input_control_identifier::kb_numpad_equals]; continue; }
        }
    }
    
    {
        bool UNKNOWN = false; // urgh: https://stackoverflow.com/questions/12536356/how-to-detect-key-up-or-key-release-for-the-capslock-key-in-os-x
        
        const bool caps_lk_locked = UNKNOWN;
        const bool caps_lk_pressed = g_keyboard_pressed_fns.find (NSEventModifierFlagCapsLock) != g_keyboard_pressed_fns.end();
        if (caps_lk_locked || caps_lk_pressed)
            input[sge::input_control_identifier::kq_caps_lk] = std::make_pair (caps_lk_locked, caps_lk_pressed);
        
        const bool scr_lk_locked = UNKNOWN;
        const bool scr_lk_pressed = g_keyboard_pressed_fns.find (NSScrollLockFunctionKey) != g_keyboard_pressed_fns.end();
        if (scr_lk_locked || scr_lk_pressed)
            input[sge::input_control_identifier::kq_scr_lk] = std::make_pair (scr_lk_locked, scr_lk_pressed);

        const bool num_lk_locked = UNKNOWN;
        const bool num_lk_pressed = UNKNOWN;
        if (num_lk_locked || num_lk_pressed)
            input[sge::input_control_identifier::kq_num_lk] = std::make_pair (num_lk_locked, num_lk_pressed);
    }

    
    // mouse
    input[sge::input_control_identifier::md_scrollwheel] = g_mouse_scrollwheel;
    input[sge::input_control_identifier::mp_position] = sge::input_point_control { (int) g_mouse_position_x, (int) g_mouse_position_y };
    if (g_mouse_left) input[sge::input_control_identifier::mb_left] = true;
    if (g_mouse_middle) input[sge::input_control_identifier::mb_middle] = true;
    if (g_mouse_right) input[sge::input_control_identifier::mb_right] = true;
    
    // gamepad
    
    input[sge::input_control_identifier::ga_left_trigger] = g_gamepad.get_left_trigger ();
    input[sge::input_control_identifier::ga_right_trigger] = g_gamepad.get_right_trigger ();
    auto gamepad_left_stick = g_gamepad.get_left_stick ();
    input[sge::input_control_identifier::ga_left_stick_x] = gamepad_left_stick.x;
    input[sge::input_control_identifier::ga_left_stick_y] = gamepad_left_stick.y;
    auto gamepad_right_stick = g_gamepad.get_right_stick ();
    input[sge::input_control_identifier::ga_right_stick_x] = gamepad_right_stick.x;
    input[sge::input_control_identifier::ga_right_stick_y] = gamepad_right_stick.y;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::dpad_up)) input[sge::input_control_identifier::gb_dpad_up] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::dpad_down)) input[sge::input_control_identifier::gb_dpad_down] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::dpad_left)) input[sge::input_control_identifier::gb_dpad_left] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::dpad_right)) input[sge::input_control_identifier::gb_dpad_right] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::option_left)) input[sge::input_control_identifier::gb_back] = true;
    //if (g_gamepad.is_button_pressed (iokit_gamepad::button::option_middle)) input[sge::input_control_identifier::gb_center] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::option_right)) input[sge::input_control_identifier::gb_start] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::left_thumb)) input[sge::input_control_identifier::gb_left_thumb] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::right_thumb)) input[sge::input_control_identifier::gb_right_thumb] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::left_shoulder)) input[sge::input_control_identifier::gb_left_shoulder] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::right_shoulder)) input[sge::input_control_identifier::gb_right_shoulder] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::action_south)) input[sge::input_control_identifier::gb_a] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::action_east)) input[sge::input_control_identifier::gb_b] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::action_west)) input[sge::input_control_identifier::gb_x] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::action_north)) input[sge::input_control_identifier::gb_y] = true;
    
}

@implementation NSWindow (TitleBarHeight)

- (CGFloat) titlebarHeight
{
    CGFloat contentHeight = [self contentRectForFrameRect: self.frame].size.height;
    return self.frame.size.height - contentHeight;
}

@end

@interface metal_view_controller : NSViewController<MTKViewDelegate>
- (void) loadView;
- (void) setWindow: (NSWindow *) _;
@end

@implementation metal_view_controller {
    sge::app::configuration* _configuration;
    MTKView* _view;
    NSWindow * _window;
}

- (void)loadView {
    _configuration = &sge::app::get_configuration ();
    self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, _configuration->app_width, _configuration->app_height)];
    g_container_width = _configuration->app_width;
    g_container_height = _configuration->app_height;
}

- (void) setWindow: (NSWindow *) w {
    _window = w;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    [self.view.window makeFirstResponder:self];
    [self.view.window makeKeyAndOrderFront:self];
    _view = (MTKView *) self.view;
    //CAMetalLayer *metalLayer = (CAMetalLayer*)_view.layer;
    
    _view.delegate = self;
    
    g_sge->setup (self.view);

    g_sge->register_set_window_title_callback ([self](const char* s) {
        [_window setTitle:[NSString stringWithUTF8String:s]];
    });
    g_sge->register_set_window_fullscreen_callback ([self](bool v) {
        bool is_fullscreen = ((_window.styleMask & NSWindowStyleMaskFullScreen) == NSWindowStyleMaskFullScreen);
        
        if (is_fullscreen) {
            [_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenNone];
        } else {
            [_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
        }
        if(is_fullscreen != v) {
             [_window toggleFullScreen:nil];
        }
        
        if (is_fullscreen && g_target_window_size.has_value ()) {

            NSRect frame = [_window frame];
            frame.size.width = g_target_window_size.value().x;
            frame.size.height = g_target_window_size.value().y + _window.titlebarHeight;
            [_window setFrame: frame display: YES animate: false];
            
            g_target_window_size.reset ();
        }

    });
    g_sge->register_set_window_size_callback ([self](int w, int h) {
        bool is_fullscreen = ((_window.styleMask & NSWindowStyleMaskFullScreen) == NSWindowStyleMaskFullScreen);
        if(is_fullscreen) {
            g_target_window_size = { w, h };
            return;
        }
        NSRect frame = [_window frame];
        frame.size.width = w;
        frame.size.height = h + _window.titlebarHeight;
        [_window setFrame: frame display: YES animate: false];
    });
    g_sge->register_shutdown_request_callback ([self]() {
        [NSApp terminate:self];
    });
    

    g_sge->start ();
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize) size {
    g_is_resizing = true;
    g_container_width = size.width;
    g_container_height = size.height;
}

- (void)drawInMTKView:(nonnull MTKView *) view {

    // update application side stuff
    NSPoint point = [[view window] mouseLocationOutsideOfEventStream];
    g_mouse_position_x = std::clamp <int>(point.x, 0, g_container_width);
    g_mouse_position_y = g_container_height - std::clamp<int> (point.y, 0, g_container_height);
    
    //g_gamepad.update();
    g_gamepad.update();

    // update the engine
    sge::core::container_state container_state;
    sge::input_state input_state;

    calculate_sge_container_state (container_state);
    calculate_sge_input_state (input_state);

    g_sge->update (container_state, input_state);
    
    g_is_resizing = false;
}

- (void)mouseDown:(NSEvent *)           _ { g_mouse_left = true; }
- (void)mouseUp:(NSEvent *)             _ { g_mouse_left = false; }
- (void)otherMouseDown:(NSEvent *)      _ { g_mouse_middle = true; }
- (void)otherMouseUp:(NSEvent *)        _ { g_mouse_middle = false; }
- (void)rightMouseDown:(NSEvent *)      _ { g_mouse_right = true; }
- (void)rightMouseUp:(NSEvent *)        _ { g_mouse_right = false; }
- (void)scrollWheel:(NSEvent *)         e { g_mouse_scrollwheel += e.scrollingDeltaY * 10.0f; }
- (void)flagsChanged:(NSEvent *)        e {
    if ([e modifierFlags] & NSEventModifierFlagCapsLock)    g_keyboard_pressed_fns.insert (NSEventModifierFlagCapsLock);
    else                                                    g_keyboard_pressed_fns.erase  (NSEventModifierFlagCapsLock);
    if ([e modifierFlags] & NSEventModifierFlagControl)     g_keyboard_pressed_fns.insert (NSEventModifierFlagControl);
    else                                                    g_keyboard_pressed_fns.erase  (NSEventModifierFlagControl);
    if ([e modifierFlags] & NSEventModifierFlagShift) {
        g_keyboard_pressed_fns.insert (NSEventModifierFlagShift);
#if SGE_OSX_INPUT_DEBUG
        std::cout << "shift down" << std::endl;
        //g_keyboard_pressed_characters.clear();
#endif
    }
    else {
        g_keyboard_pressed_fns.erase  (NSEventModifierFlagShift);
#if SGE_OSX_INPUT_DEBUG
        std::cout << "shift up" << std::endl;
        //g_keyboard_pressed_characters.clear();
#endif
    }
    if ([e modifierFlags] & NSEventModifierFlagOption)      g_keyboard_pressed_fns.insert (NSEventModifierFlagOption);
    else                                                    g_keyboard_pressed_fns.erase  (NSEventModifierFlagOption);
    if ([e modifierFlags] & NSEventModifierFlagCommand)     g_keyboard_pressed_fns.insert (NSEventModifierFlagCommand);
    else                                                    g_keyboard_pressed_fns.erase  (NSEventModifierFlagCommand);
    if ([e modifierFlags] & NSEventModifierFlagNumericPad)  g_keyboard_pressed_fns.insert (NSEventModifierFlagNumericPad);
    else                                                    g_keyboard_pressed_fns.erase  (NSEventModifierFlagNumericPad);
    
}
-(void)keyDown:(NSEvent *) e {
    if ([e isARepeat])
        return;
    
    unsigned short characterHit = [[e charactersIgnoringModifiers] characterAtIndex:0];
    
    if ([e modifierFlags] & NSEventModifierFlagShift) {
        ;
    }
    
#if SGE_OSX_INPUT_DEBUG
    std::cout << "key down: " << characterHit << std::endl;
#endif

    g_keyboard_pressed_characters.erase (tolower (characterHit));
    g_keyboard_pressed_characters.erase (toupper (characterHit));
    g_keyboard_pressed_characters.insert (characterHit);

    
    if ((characterHit >= 0xF700 && characterHit <= 0xF8FF)
        || characterHit == 13 // enter
        || characterHit == 32 // spacebar
        || characterHit == 127 // backspace
        || characterHit == 27 // escape
        || characterHit == 9 // tab
        || characterHit == 3 // numpad_enter
    ){
        g_keyboard_pressed_fns.insert (characterHit);
    }
}


-(void)keyUp:(NSEvent *) e {
    if ([e isARepeat])
        return;

    unsigned short characterHit = [[e charactersIgnoringModifiers] characterAtIndex:0];

    if ([e modifierFlags] & NSEventModifierFlagShift) {
        ;
    }
    
#if SGE_OSX_INPUT_DEBUG
    std::cout << "key up: " << characterHit << std::endl;
#endif

    g_keyboard_pressed_characters.erase (tolower (characterHit));
    g_keyboard_pressed_characters.erase (toupper (characterHit));

    if ((characterHit >= 0xF700 && characterHit <= 0xF8FF)
        || characterHit == 13 // enter
        || characterHit == 32 // spacebar
        || characterHit == 127 // backspace
        || characterHit == 27 // escape
        || characterHit == 9 // tab
        || characterHit == 3 // numpad_enter
    ){
        g_keyboard_pressed_fns.erase (characterHit);
    }
}
@end

//################################################################################################//

@interface app_delegate : NSObject <NSApplicationDelegate>
@end

@implementation app_delegate {
    NSViewController * _controller;
    NSWindow * _window;
}
- (id)init {
    _controller = [[metal_view_controller alloc] initWithNibName:nil bundle:nil];
    auto sm = NSWindowStyleMaskTitled | NSWindowStyleMaskResizable;
    _window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 800, 600) styleMask:sm backing:NSBackingStoreBuffered defer:NO];
    
    [_window makeFirstResponder:_controller];
    
    // only allow fullscreen to be enabled from the engine (not from the green window button) as the
    // engine does not query the host's full screen state and assumes it doesn't change beneath its feet.
    [_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenNone];
    
    metal_view_controller *controller = (metal_view_controller*)_controller;
    [controller setWindow: _window];
    
    return self;
}
- (void)dealloc {
    [_window release];
    [_controller release];
    [super dealloc];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    id appName = [[NSProcessInfo processInfo] processName];
    [_window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
    [_window setTitle:appName];
    [_window makeKeyAndOrderFront:self];
    [_window setContentViewController:_controller];
    
    [[_window standardWindowButton:NSWindowCloseButton] setHidden:NO];
    [[_window standardWindowButton:NSWindowMiniaturizeButton] setHidden:NO];
    [[_window standardWindowButton:NSWindowZoomButton] setHidden:NO];
    
    [[_window standardWindowButton:NSWindowCloseButton] setEnabled:YES];
    [[_window standardWindowButton:NSWindowMiniaturizeButton] setEnabled:NO];
    [[_window standardWindowButton:NSWindowZoomButton] setEnabled:YES];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
	return YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    g_sge->stop ();
    g_sge->shutdown();
}
@end


//################################################################################################//

int main ()
{
    [NSAutoreleasePool new];
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];
    id appMenu = [[NSMenu new] autorelease];
    id quitTitle = @"Quit";
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
    id appDelegate = [[app_delegate new] autorelease];
    [NSApp setDelegate:appDelegate];
    [NSApp activateIgnoringOtherApps:YES];
    g_sge = std::make_unique<sge::core::engine>();
    [NSApp run];
    g_sge.reset();
    return 0;

}

#endif
