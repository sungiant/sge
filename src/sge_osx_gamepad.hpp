#if TARGET_MACOSX

// SGE-OSX-GAMEPAD
// -------------------------------------
// Standalone IOKit gamepad cobbled
// together from bits on the internet.
// -------------------------------------

#pragma once

#include <IOKit/hid/IOHIDManager.h>
#include <vector>
#include <queue>
#include <string>
#include <functional>
#include <chrono>
#include <unordered_set>

// Tested against:
// * N30 Pro
// * Wired PS4 DualShock
class iokit_gamepad {
public:
    struct vector { float x; float y; };
    enum class button { dpad_up, dpad_down, dpad_left, dpad_right, start, back, left_thumb, right_thumb, left_shoulder, right_shoulder, a, b, x, y };

    iokit_gamepad ();
    ~iokit_gamepad ();

    void update ();

    vector get_left_stick () const { return state.left_stick; }
    vector get_right_stick () const { return state.right_stick; }
    float get_left_trigger () const { return state.left_trigger; }
    float get_right_trigger () const { return state.right_trigger; }
    bool is_button_pressed (button z) const { return state.pressed_buttons.find (z) != state.pressed_buttons.end (); }

private:

    struct {
        std::unordered_set<button> pressed_buttons;
        vector left_stick;
        vector right_stick;
        float left_trigger;
        float right_trigger;
    } state;

    static void* thread_start (void*);
    static void iohid_attached_callback (void*, IOReturn, void*, IOHIDDeviceRef);
    static void iohid_detached_callback (void*, IOReturn, void*, IOHIDDeviceRef);
    static void iohid_input_callback (void*, IOReturn, void*, IOHIDValueRef);
    
    struct Device {
      unsigned int device_id = 0;
      int vendor_id = 0;
      int product_id = 0;
      std::string description;
      std::vector<float> axes;
      std::vector<bool> buttons;
    };

    struct HidButtonInfo {
      int button_id = -1;
      int cookie = -1;
    };

    struct HidAxisInfo {
      int axis_id = -1;
      int cookie = -1;
      int minimum = 0;
      int maximum = 0;
      int fuzz = 0;
      int flat = 0;
    };

    struct HidCookieInfo {
      int axis_id = -1;
      int button_id = -1;
    };

    struct HidDevice {
      IOHIDDeviceRef device_ref = nullptr;
      iokit_gamepad* parent = nullptr;
      bool disconnected = false;
      Device device;
      std::vector<HidButtonInfo> button_infos;
      std::vector<HidAxisInfo> axis_infos;
      std::vector<HidCookieInfo> cookie_map;
    };

    struct HidEvent {
      HidDevice* device = nullptr;
      int axis_id = -1;
      int button_id = -1;
      int value;
    };

    void init ();
    void process ();
    void uninitialise_device (HidDevice* device);

    bool                        initialized_ = false;
    int                         next_device_id_ = 0;
    IOHIDManagerRef             hid_manager_ = nullptr;
    std::vector<HidDevice*>     devices_;

    pthread_t                   event_thread_;
    CFRunLoopRef                event_thread_loop_ = nullptr;
    
    std::queue<HidEvent>        event_queue_;
    pthread_mutex_t             event_queue_mutex_;
    
    void on_device_attached (Device*);
    void on_device_detached (Device*);
    void on_device_button_pressed (Device*, int);
    void on_device_button_released (Device*, int);
    void on_device_axis_moved (Device*, int, float, float);
    
    std::chrono::high_resolution_clock::time_point     last_gamepad_update;
    std::chrono::high_resolution_clock::time_point     last_gamepad_scan;
    
};

#endif
