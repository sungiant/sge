// SGE-OSX
// Reference SGE host implementation.
// ---------------------------------- //

#if TARGET_MACOSX

#include <pthread.h>

#include <chrono>
#include <unordered_set>
#include <algorithm>
#include <optional>
#include <vector>
#include <queue>
#include <functional>
#include <string>
#include <thread>

#include <Cocoa/Cocoa.h>
#include <MetalKit/MTKView.h>
#include <IOKit/hid/IOHIDManager.h>

#include "sge.hh"
#include "sge_core.hh"

#define SGE_OSX_INPUT_DEBUG 1



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

    enum identifier_type { BUTTON, AXIS };

    enum button_cookie {
        BUTTON_ACTION_SOUTH = 0, BUTTON_ACTION_EAST = 1,
        BUTTON_ACTION_WEST = 2, BUTTON_ACTION_NORTH = 3,
        BUTTON_LEFT_SHOULDER = 4, BUTTON_RIGHT_SHOULDER = 5,
        BUTTON_LEFT_TRIGGER = 6, BUTTON_RIGHT_TRIGGER = 7,
        BUTTON_OPTION_LEFT = 8, BUTTON_OPTION_RIGHT = 9,
        BUTTON_LEFT_THUMB = 10, BUTTON_RIGHT_THUMB = 11,
        BUTTON_OPTION_EXTRA = 12, BUTTON_OPTION_MIDDLE = 13,
    };
        
    enum axis_cookie {
        AXIS_LEFT_X = 0, AXIS_LEFT_Y = 1, AXIS_RIGHT_X = 2, AXIS_RIGHT_Y = 3,
        AXIS_DPAD = 4, AXIS_LEFT_TRIGGER = 5, AXIS_RIGHT_TRIGGER = 6,
    };

    struct attached_event   { IOHIDDeviceRef device; };
    struct detached_event   { IOHIDDeviceRef device; };
    struct input_event      { IOHIDDeviceRef device;
                              IOHIDElementCookie element_cookie; IOHIDElementType element_type;
                              identifier_type identifier_type; uint32_t identifier; int value; };

    struct device_state {
        std::unordered_set<button>  pressed_buttons;
        vector                      left_stick;
        vector                      right_stick;
        float                       left_trigger;
        float                       right_trigger;
    };

    struct device_info {
        std::string product;
        std::string manufacturer;
        int vendor_id;
        int product_id;
        std::unordered_map<IOHIDElementCookie, int> button_indicies;
        std::unordered_map<IOHIDElementCookie, int> axis_indicies;
    };

    struct input_reference {
        input_reference (iokit_gamepad& zp, IOHIDDeviceRef zd, device_info& zi) : parent (zp), device (zd), info (zi) {}
        iokit_gamepad&              parent;
        const IOHIDDeviceRef        device;
        device_info                 info;

    };

    typedef std::variant<attached_event, detached_event, input_event>       event;
    typedef std::chrono::high_resolution_clock::time_point                  timestamp;
    typedef std::optional<std::pair<IOHIDDeviceRef, device_state>>          connection;

    // accessed on hid thread
    std::queue<event>                                                       event_queue;
    std::unordered_map<IOHIDDeviceRef, std::unique_ptr<input_reference>>    input_callback_data;

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
            constexpr int page_desktop     = kHIDPage_GenericDesktop;
            constexpr int usage_joystick   = kHIDUsage_GD_Joystick;
            constexpr int usage_gamepad    = kHIDUsage_GD_GamePad;
            constexpr int usage_controller = kHIDUsage_GD_MultiAxisController;
            CFStringRef keys[] { CFSTR (kIOHIDDeviceUsagePageKey), CFSTR (kIOHIDDeviceUsageKey) };
            const auto desktop    = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &page_desktop);
            const auto joystick   = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &usage_joystick);
            const auto gamepad    = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &usage_gamepad);
            const auto controller = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &usage_controller);
            const std::vector<CFNumberRef> desktop_joystick   { desktop, joystick };
            const std::vector<CFNumberRef> desktop_gamepad    { desktop, gamepad };
            const std::vector<CFNumberRef> desktop_controller { desktop, controller };
            const std::vector<CFDictionaryRef> devices {
                CFDictionaryCreate (kCFAllocatorDefault, (const void **) keys, (const void **) desktop_joystick.data(),   desktop_joystick.size(),   &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks),
                CFDictionaryCreate (kCFAllocatorDefault, (const void **) keys, (const void **) desktop_gamepad.data(),    desktop_gamepad.size(),    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks),
                CFDictionaryCreate (kCFAllocatorDefault, (const void **) keys, (const void **) desktop_controller.data(), desktop_controller.size(), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
            CFRelease (controller); CFRelease (gamepad); CFRelease (joystick); CFRelease (desktop);
            const CFArrayRef multiple = CFArrayCreate (kCFAllocatorDefault, (const void **) devices.data(), devices.size(), &kCFTypeArrayCallBacks);
            std::for_each (devices.begin(), devices.end(), [](CFDictionaryRef r){ CFRelease (r); });
            IOHIDManagerSetDeviceMatchingMultiple (hid_manager, multiple);
            CFRelease (multiple);
        }
        IOHIDManagerRegisterDeviceMatchingCallback (hid_manager, iohid_attached_callback, this);
        IOHIDManagerRegisterDeviceRemovalCallback  (hid_manager, iohid_detached_callback, this);
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
            if (connections[i].has_value ())
                remove_connection (connections[i].value().first);
        }
        pthread_mutex_lock (&mutex);
        while (event_queue.size ())
            event_queue.pop();
        
        // having this bit causes `IOHIDManagerClose` to assert... :/
        //for (auto& kvp : input_callback_data) {
            //IOReturn r = IOHIDDeviceClose (kvp.first, kIOHIDOptionsTypeSeizeDevice);
            //if (r != kIOReturnSuccess) {
            //    std::cout << "Unexpected return code [" << r << "] from IOHIDDeviceClose (" << kvp.first << ")" << '\n';
            //}
        //}

        input_callback_data.clear ();
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
                if (input_event* input = std::get_if<input_event> (&event)) { handle_input_event (input->device, input->identifier_type, input->identifier, input->value); }
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

    void handle_input_event (IOHIDDeviceRef device, identifier_type type, int identifier, int value) {

        const auto idx = get_connection_index (device);
        assert (idx.has_value());
        const int index = idx.value();
        assert (connections[index].has_value());
        auto& state = connections[index].value ().second;
        
        if (type == identifier_type::BUTTON) {
            switch (identifier) {
                #define CASE(x, y) { case BUTTON_ ## x: { if (value > 0) state.pressed_buttons.insert (y); else state.pressed_buttons.erase (y); break; } }
                CASE (ACTION_SOUTH,   button::action_south);
                CASE (ACTION_EAST,    button::action_east);
                CASE (ACTION_WEST,    button::action_west);
                CASE (ACTION_NORTH,   button::action_north);
                CASE (LEFT_SHOULDER,  button::left_shoulder);
                CASE (RIGHT_SHOULDER, button::right_shoulder);
                CASE (OPTION_LEFT,    button::option_left);
                CASE (OPTION_MIDDLE,  button::option_middle);
                CASE (OPTION_RIGHT,   button::option_right);
                CASE (LEFT_THUMB,     button::left_thumb);
                CASE (RIGHT_THUMB,    button::right_thumb);
                #undef CASE
                // button triggers (these exist on the N30 Pro instead of real triggers)
                case BUTTON_LEFT_TRIGGER: { if (value > 0) state.left_trigger = 1.0f; else state.left_trigger = 0.0f; break; }
                case BUTTON_RIGHT_TRIGGER: { if (value > 0) state.right_trigger = 1.0f; else state.right_trigger = 0.0f; break; }
            }
        }
        
        if (type == identifier_type::AXIS) {
            switch (identifier) {
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
                }
                // triggers
                case AXIS_LEFT_TRIGGER: case AXIS_RIGHT_TRIGGER: {
                    float v = (float) value / 255.0f;
                    const float TRIGGER_DEADZONE = 0.01f;
                    if (value < TRIGGER_DEADZONE) { v = 0.0f; }
                    if (identifier == AXIS_LEFT_TRIGGER)      state.left_trigger = v;
                    else if (identifier == AXIS_RIGHT_TRIGGER) state.right_trigger = v;
                    break;
                }
            }
        }
    }

    static void get_device_info (IOHIDDeviceRef device, device_info& info) {
        
        info.vendor_id = get_registry_int (device, CFSTR (kIOHIDVendorIDKey));
        info.product_id = get_registry_int (device, CFSTR (kIOHIDProductIDKey));
        info.product = get_registry_string (device, CFSTR (kIOHIDProductKey));
        info.manufacturer = get_registry_string (device, CFSTR (kIOHIDManufacturerKey));
        
        
        CFArrayRef elements = IOHIDDeviceCopyMatchingElements (device, nullptr, kIOHIDOptionsTypeNone);
        for (int i = 0; i < CFArrayGetCount (elements); i++) {
            IOHIDElementRef element = (IOHIDElementRef)CFArrayGetValueAtIndex (elements, i);
            IOHIDElementType type = IOHIDElementGetType (element);

            const uint32_t usagePage = IOHIDElementGetUsagePage (element);
            if (usagePage != kHIDPage_GenericDesktop && usagePage != kHIDPage_Button) {
                continue;
            }

            IOHIDElementCookie cookie = IOHIDElementGetCookie (element);
            if (type == kIOHIDElementTypeInput_Misc ||
                type == kIOHIDElementTypeInput_Axis) {
                CFIndex minimum = IOHIDElementGetLogicalMin (element);
                CFIndex maximum = IOHIDElementGetLogicalMax (element);
                std::cout << "adding axis [" << cookie << "] #" << info.axis_indicies.size() << " (" << minimum << " - " << maximum << ")" << '\n';
                info.axis_indicies[cookie] = info.axis_indicies.size();
            }
            else if (type == kIOHIDElementTypeInput_Button) {
                std::cout << "adding button [" << cookie << "] #" << info.button_indicies.size() << '\n';
                info.button_indicies[cookie] = info.button_indicies.size();
            }
        }
        CFRelease (elements);
    }
    static int get_registry_int (IOHIDDeviceRef device, CFStringRef k) {
        int r;
        CFTypeRef rr = IOHIDDeviceGetProperty (device, k);
        assert (CFGetTypeID (rr) == CFNumberGetTypeID ());
        CFNumberGetValue ((CFNumberRef) rr, kCFNumberSInt32Type, &r);
        return r;
    }
    static std::string get_registry_string (IOHIDDeviceRef device, CFStringRef k) {
        std::string r;
        CFTypeRef rr = IOHIDDeviceGetProperty (device, k);
        if (rr == nullptr || CFGetTypeID (rr) != CFStringGetTypeID ()) {
            r = "<unknown>";
        } else {
            char buffer[1024];
            CFStringGetCString ((CFStringRef)rr, buffer, 1024, kCFStringEncodingUTF8);
            r = buffer;
        }
        return r;
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
        
        device_info info = {};
        get_device_info(device, info);
        //bool ps4_dual_shock = (info.vendor_id == 1356 && info.product_id == 1476);

        //std::cout << "[T] Opening device (" << device << "):" << info.manufacturer << " " << info.product << '\n';
        //IOReturn r = IOHIDDeviceOpen (device, kIOHIDOptionsTypeNone); // kIOHIDOptionsTypeSeizeDevice
        //if (r != kIOReturnSuccess) {
        //    std::cout << "Unexpected return code [" << r << "] from IOHIDDeviceOpen (" << device << ")" << '\n';
        //}
        
        pthread_mutex_lock (&gp->mutex);
        gp->input_callback_data[device] = std::make_unique<input_reference> (*gp, device, info);
        gp->event_queue.push (attached_event { device });
        IOHIDDeviceRegisterInputValueCallback (device, iohid_input_callback, gp->input_callback_data[device].get());
        IOHIDDeviceScheduleWithRunLoop (device, gp->event_thread_loop, kCFRunLoopDefaultMode);
        pthread_mutex_unlock (&gp->mutex);
        

    }

    static void iohid_detached_callback (void* context, IOReturn result, void* sender, IOHIDDeviceRef device) {
        //std::cout << "[T] Closing device (" << device << ")" << '\n';
        //IOReturn r = IOHIDDeviceClose (device, kIOHIDOptionsTypeNone);
        //if (r != kIOReturnSuccess) {
        //    std::cout << "Unexpected return code [" << r << "] from IOHIDDeviceClose (" << device << ")." << '\n';
        //}
        
        iokit_gamepad* gp = static_cast<iokit_gamepad*>(context);
        pthread_mutex_lock (&gp->mutex);
        gp->input_callback_data.erase (device);
        gp->event_queue.push (detached_event { device });
        pthread_mutex_unlock (&gp->mutex);
    }

    static void iohid_input_callback (void* context, IOReturn result, void* sender, IOHIDValueRef value) {
       
        
        input_reference* ir = static_cast<input_reference*>(context);
        
        const IOHIDElementRef element = IOHIDValueGetElement (value);
        IOHIDElementType element_type = IOHIDElementGetType (element);
        const IOHIDElementCookie element_cookie = IOHIDElementGetCookie (element);
        const int int_value = IOHIDValueGetIntegerValue (value);
        
        pthread_mutex_lock (&ir->parent.mutex);

        if (ir->parent.event_queue.size () < 512) {
            if (ir->info.button_indicies.find (element_cookie) != ir->info.button_indicies.end()) {
                assert (element_type == kIOHIDElementTypeInput_Button);
                uint32_t identifier = ir->info.button_indicies.at (element_cookie);
                auto item = input_event { ir->device, element_cookie, element_type, identifier_type::BUTTON, identifier, int_value };
                ir->parent.event_queue.push (item);
            }
            if (ir->info.axis_indicies.find (element_cookie) != ir->info.axis_indicies.end()) {
                assert (element_type == kIOHIDElementTypeInput_Axis || element_type == kIOHIDElementTypeInput_Misc);
                uint32_t identifier = ir->info.axis_indicies.at (element_cookie);
                auto item = input_event { ir->device, element_cookie, element_type, identifier_type::AXIS, identifier, int_value };
                ir->parent.event_queue.push (item);
            }
        }
        else assert (false); // too many events!
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
    #define IF(x, y) { if (g_gamepad.is_button_pressed (x)) input[y] = true; }
    IF (iokit_gamepad::button::dpad_up,         sge::input_control_identifier::gb_dpad_up);
    IF (iokit_gamepad::button::dpad_down,       sge::input_control_identifier::gb_dpad_down);
    IF (iokit_gamepad::button::dpad_left,       sge::input_control_identifier::gb_dpad_left);
    IF (iokit_gamepad::button::dpad_right,      sge::input_control_identifier::gb_dpad_right);
    IF (iokit_gamepad::button::option_left,     sge::input_control_identifier::gb_back);
    //IF (iokit_gamepad::button::option_middle, sge::input_control_identifier::gb_center);
    IF (iokit_gamepad::button::option_right,    sge::input_control_identifier::gb_start);
    IF (iokit_gamepad::button::left_thumb,      sge::input_control_identifier::gb_left_thumb);
    IF (iokit_gamepad::button::right_thumb,     sge::input_control_identifier::gb_right_thumb);
    IF (iokit_gamepad::button::left_shoulder,   sge::input_control_identifier::gb_left_shoulder);
    IF (iokit_gamepad::button::right_shoulder,  sge::input_control_identifier::gb_right_shoulder);
    IF (iokit_gamepad::button::action_south,    sge::input_control_identifier::gb_a);
    IF (iokit_gamepad::button::action_east,     sge::input_control_identifier::gb_b);
    IF (iokit_gamepad::button::action_west,     sge::input_control_identifier::gb_x);
    IF (iokit_gamepad::button::action_north,    sge::input_control_identifier::gb_y);
    #undef IF
    
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

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender { return YES; }

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    g_sge->stop ();
    g_sge->shutdown();
    g_sge.reset();
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
    // we never get here.
    assert (false);
    g_sge.reset();
    return 0;

}

#endif
