#if TARGET_MACOSX

#include "sge_osx_gamepad.hpp"

#include <thread>
#include <pthread.h>
#include <iostream>

#include "sge_math.hpp"

#define RUN_LOOP_MODE_DISCOVERY CFSTR ("RunLoopModeDiscovery")

constexpr int kHidPageDesktop = kHIDPage_GenericDesktop;
constexpr int kHidUsageGamepad = kHIDUsage_GD_GamePad;
constexpr int kHidUsageJoystick = kHIDUsage_GD_Joystick;
constexpr int kHidUsageController = kHIDUsage_GD_MultiAxisController;

void iokit_gamepad::iohid_attached_callback (void* context, IOReturn result, void* sender, IOHIDDeviceRef device) {
    CFTypeRef vendorRef = IOHIDDeviceGetProperty (device, CFSTR (kIOHIDVendorIDKey));
    CFTypeRef productRef = IOHIDDeviceGetProperty (device, CFSTR (kIOHIDVendorIDKey));
    if (vendorRef == nullptr || productRef == nullptr || CFGetTypeID (vendorRef) != CFNumberGetTypeID () || CFGetTypeID (productRef) != CFNumberGetTypeID ()) {
        std::cerr << "Error: Vendor or Product ID not numbers!" << std::endl;
        return;
    }
    int vendor_id, product_id;
    CFNumberGetValue ((CFNumberRef)vendorRef, kCFNumberSInt32Type, &vendor_id);
    CFNumberGetValue ((CFNumberRef)productRef, kCFNumberSInt32Type, &product_id);

    std::string device_name;
    CFTypeRef nameRef = IOHIDDeviceGetProperty (device, CFSTR (kIOHIDProductKey));
    if (nameRef == nullptr || CFGetTypeID (nameRef) != CFStringGetTypeID ()) {
        device_name = "<Unknown>";
    } else {
        char buffer[1024];
        CFStringGetCString ((CFStringRef)nameRef, buffer, 1024, kCFStringEncodingUTF8);
        device_name = buffer;
    }
    
    iokit_gamepad* gp = static_cast<iokit_gamepad*> (context);

    HidDevice* hid_device = new HidDevice ();
    hid_device->device_ref = device;
    hid_device->parent = gp;
    hid_device->device.vendor_id = vendor_id;
    hid_device->device.product_id = product_id;
    hid_device->device.description = device_name;

    int max_cookie_id = 0;
    CFArrayRef elements = IOHIDDeviceCopyMatchingElements (device, nullptr, kIOHIDOptionsTypeNone);
    for (int i = 0; i < CFArrayGetCount (elements); i++) {
        IOHIDElementRef element = (IOHIDElementRef)CFArrayGetValueAtIndex (elements, i);
        IOHIDElementType type = IOHIDElementGetType (element);

        const uint32_t usagePage = IOHIDElementGetUsagePage (element);
        if (usagePage != kHIDPage_GenericDesktop && usagePage != kHIDPage_Button) {
            continue;
        }

        if (type == kIOHIDElementTypeInput_Button) {
            HidButtonInfo button_info;
            button_info.button_id = hid_device->button_infos.size ();
            button_info.cookie = IOHIDElementGetCookie (element);
            hid_device->button_infos.push_back (button_info);
            max_cookie_id = std::max (max_cookie_id, button_info.cookie);
        }
        else if (type == kIOHIDElementTypeInput_Misc ||
            type == kIOHIDElementTypeInput_Axis) {
            HidAxisInfo axis_info;
            axis_info.axis_id = hid_device->axis_infos.size ();
            axis_info.cookie = IOHIDElementGetCookie (element);
            axis_info.minimum = IOHIDElementGetLogicalMin (element);
            axis_info.maximum = IOHIDElementGetLogicalMax (element);
            hid_device->axis_infos.push_back (axis_info);
            max_cookie_id = std::max (max_cookie_id, axis_info.cookie);
        }
    }
    CFRelease (elements);

    hid_device->device.buttons.resize (hid_device->button_infos.size (), false);
    hid_device->device.axes.resize (hid_device->axis_infos.size (), 0.0f);

    hid_device->cookie_map.resize (max_cookie_id + 1);
    for (const HidButtonInfo& button : hid_device->button_infos) {
        hid_device->cookie_map[button.cookie].button_id = button.button_id;
    }
    for (const HidAxisInfo& axis : hid_device->axis_infos) {
        hid_device->cookie_map[axis.cookie].axis_id = axis.axis_id;
    }

    hid_device->device.device_id = gp->next_device_id_++;
    gp->devices_.push_back (hid_device);
    gp->on_device_attached (&gp->devices_.back ()->device);

    IOHIDDeviceOpen (device, kIOHIDOptionsTypeNone);
    IOHIDDeviceRegisterInputValueCallback (device, iohid_input_callback, gp->devices_.back ());
    IOHIDDeviceScheduleWithRunLoop (device, gp->event_thread_loop_, kCFRunLoopDefaultMode);
}

void iokit_gamepad::iohid_detached_callback (void* context, IOReturn result, void* sender, IOHIDDeviceRef device) {
    iokit_gamepad* gp = static_cast<iokit_gamepad*>(context);
    for (HidDevice* hid_device : gp->devices_) {
        if (hid_device->device_ref == device) {
            gp->uninitialise_device (hid_device);
            return;
        }
    }
}

void iokit_gamepad::iohid_input_callback (void* context, IOReturn result, void* sender, IOHIDValueRef value) {
    HidDevice* device = static_cast<HidDevice*>(context);

    if (device == nullptr || value == nullptr)
        return;

    IOHIDElementRef element = IOHIDValueGetElement (value);
    IOHIDElementCookie cookie = IOHIDElementGetCookie (element);
    const int int_value = IOHIDValueGetIntegerValue (value);

    if (cookie >= device->cookie_map.size ())
        return;
        
    const HidCookieInfo& cookie_info = device->cookie_map[cookie];
    if (cookie_info.axis_id < 0 && cookie_info.button_id < 0)
        return;
    
    HidEvent event;
    event.device = device;
    event.axis_id = cookie_info.axis_id;
    event.button_id = cookie_info.button_id;
    event.value = int_value;

    iokit_gamepad* gp = device->parent;
    pthread_mutex_lock (&gp->event_queue_mutex_);
    gp->event_queue_.push (event);
    if (gp->event_queue_.size () > 1024) {
        std::cout << "Compressing event queue (size " << gp->event_queue_.size () << ")..." << std::endl;
        pthread_mutex_lock (&gp->event_queue_mutex_);
        while (!gp->event_queue_.empty ())
            gp->event_queue_.pop ();
        pthread_mutex_unlock (&gp->event_queue_mutex_);
    }
    pthread_mutex_unlock (&gp->event_queue_mutex_);
}

void* iokit_gamepad::thread_start (void* context) {
    iokit_gamepad* system = static_cast<iokit_gamepad*>(context);
    system->event_thread_loop_ = CFRunLoopGetCurrent ();
    while (true) {
        CFRunLoopRun ();
        std::this_thread::sleep_for (std::chrono::milliseconds (10));
    }
    return nullptr;
}

iokit_gamepad::iokit_gamepad () {
    pthread_mutex_init (&event_queue_mutex_, nullptr);
}

iokit_gamepad::~iokit_gamepad () {
    if (event_thread_loop_ != nullptr) {
      pthread_cancel (event_thread_);
      event_thread_loop_ = nullptr;
    }
    pthread_mutex_destroy (&event_queue_mutex_);

    for (HidDevice* device : devices_) {
      uninitialise_device (device);
      delete device;
    }
    devices_.clear ();

    if (hid_manager_ != nullptr) {
      IOHIDManagerUnscheduleFromRunLoop (hid_manager_, CFRunLoopGetCurrent (), kCFRunLoopCommonModes);
      IOHIDManagerClose (hid_manager_, kIOHIDOptionsTypeNone);
      CFRelease (hid_manager_);
      hid_manager_ = nullptr;
    }
}

void iokit_gamepad::update () {
    auto now = std::chrono::high_resolution_clock::now ();

    // scan
    if (now - last_gamepad_scan >= std::chrono::seconds (1)) {
        if (!initialized_)
            init ();
        CFRunLoopRunInMode (RUN_LOOP_MODE_DISCOVERY, 0, true);
        last_gamepad_scan = now;
    }
    
    // update
    if (now - last_gamepad_update >= std::chrono::milliseconds (5)) {
        process ();
        for (auto iter = devices_.begin (); iter != devices_.end ();) {
            HidDevice* device = *iter;
            if (device->disconnected) {
                on_device_detached (&device->device);
                delete device;
                iter = devices_.erase (iter);
            } else { iter++; }
        }
        last_gamepad_update = now;
    }
}

void iokit_gamepad::init () {
    int r = pthread_create (&event_thread_, nullptr, &thread_start, this);
    assert (r == 0);
    
    while (event_thread_loop_ == nullptr) {
        ;
    }
    
    hid_manager_ = IOHIDManagerCreate (kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (hid_manager_ == nullptr) {
        std::cout << "Error creating HID manager." << std::endl;
        return;
    }

    CFStringRef keys[2];
    keys[0] = CFSTR (kIOHIDDeviceUsagePageKey);
    keys[1] = CFSTR (kIOHIDDeviceUsageKey);

    CFDictionaryRef dictionaries[3];
    CFNumberRef values[2];
    values[0] = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &kHidPageDesktop);
    values[1] = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &kHidUsageJoystick);
    dictionaries[0] = CFDictionaryCreate (kCFAllocatorDefault, (const void **)keys, (const void **)values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease (values[0]);
    CFRelease (values[1]);

    values[0] = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &kHidPageDesktop);
    values[1] = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &kHidUsageGamepad);
    dictionaries[1] = CFDictionaryCreate (kCFAllocatorDefault, (const void **)keys, (const void **)values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease (values[0]);
    CFRelease (values[1]);

    values[0] = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &kHidPageDesktop);
    values[1] = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &kHidUsageController);
    dictionaries[2] = CFDictionaryCreate (kCFAllocatorDefault, (const void **)keys, (const void **)values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease (values[0]);
    CFRelease (values[1]);

    CFArrayRef dictionariesRef = CFArrayCreate (kCFAllocatorDefault, (const void **)dictionaries, 3, &kCFTypeArrayCallBacks);
    CFRelease (dictionaries[0]);
    CFRelease (dictionaries[1]);
    CFRelease (dictionaries[2]);

    IOHIDManagerSetDeviceMatchingMultiple (hid_manager_, dictionariesRef);
    CFRelease (dictionariesRef);

    IOHIDManagerRegisterDeviceMatchingCallback (hid_manager_, iohid_attached_callback, this);
    IOHIDManagerRegisterDeviceRemovalCallback (hid_manager_, iohid_detached_callback, this);

    assert (IOHIDManagerOpen (hid_manager_, kIOHIDOptionsTypeNone) == kIOReturnSuccess);

    IOHIDManagerScheduleWithRunLoop (hid_manager_, CFRunLoopGetCurrent (), RUN_LOOP_MODE_DISCOVERY);
    initialized_ = true;
}

void iokit_gamepad::process () {
    pthread_mutex_lock (&event_queue_mutex_);
    std::size_t num_events = event_queue_.size ();
    pthread_mutex_unlock (&event_queue_mutex_);
    while (num_events > 0) {
        pthread_mutex_lock (&event_queue_mutex_);
        HidEvent event = event_queue_.front ();
        event_queue_.pop ();
        num_events = std::min (num_events - 1, event_queue_.size ());
        pthread_mutex_unlock (&event_queue_mutex_);
        if (event.button_id >= 0) {
            const bool is_down = event.value > 0;
            
            event.device->device.buttons[event.button_id] = is_down;
            if (is_down) on_device_button_pressed (&event.device->device, event.button_id);
            else         on_device_button_released (&event.device->device, event.button_id);
        }
        else if (event.axis_id >= 0) {
            const HidAxisInfo& axis_info = event.device->axis_infos[event.axis_id];
            event.value = event.value > -axis_info.flat && event.value < axis_info.flat ? 0 : event.value;
            const float minimum = static_cast<float>(axis_info.minimum);
            const float maximum = static_cast<float>(axis_info.maximum);
            const float range = maximum - minimum;
            const float norm = (static_cast<float>(event.value) - minimum) / range;
            const float clamped = std::max (-1.0f, std::min (1.0f, 2.0f * norm - 1.0f));
            const float last = event.device->device.axes[event.axis_id];
            const float eps = static_cast<float>(2 * axis_info.fuzz) / range;
            if (clamped > last + eps || clamped < last - eps) {
                event.device->device.axes[event.axis_id] = clamped;
                on_device_axis_moved (&event.device->device, event.axis_id, clamped, event.value);
            }
        }
    }
}

void iokit_gamepad::uninitialise_device (HidDevice* device) {
    device->disconnected = true;
    if (device->device_ref != nullptr) {
        IOHIDDeviceClose (device->device_ref, kIOHIDOptionsTypeNone);
        device->device_ref = nullptr;
    }
}

void iokit_gamepad::on_device_attached (Device* device) {

}

void iokit_gamepad::on_device_detached (Device* device) {

}

void iokit_gamepad::on_device_button_pressed (Device* device, int button_id) {
    switch (button_id){
        case 0: state.pressed_buttons.insert (button::x); break;
        case 1: state.pressed_buttons.insert (button::a); break;
        case 2: state.pressed_buttons.insert (button::b); break;
        case 3: state.pressed_buttons.insert (button::y); break;
        case 4: state.pressed_buttons.insert (button::left_shoulder); break;
        case 5: state.pressed_buttons.insert (button::right_shoulder); break;
        case 8: state.pressed_buttons.insert (button::back); break;
        case 9: state.pressed_buttons.insert (button::start); break;
        case 10: state.pressed_buttons.insert (button::left_thumb); break;
        case 11: state.pressed_buttons.insert (button::right_thumb); break;
        default: break;
    };
}

void iokit_gamepad::on_device_button_released (Device* device, int button_id) {
    switch (button_id){
        case 0: state.pressed_buttons.erase (button::x); break;
        case 1: state.pressed_buttons.erase (button::a); break;
        case 2: state.pressed_buttons.erase (button::b); break;
        case 3: state.pressed_buttons.erase (button::y); break;
        case 4: state.pressed_buttons.erase (button::left_shoulder); break;
        case 5: state.pressed_buttons.erase (button::right_shoulder); break;
        case 8: state.pressed_buttons.erase (button::back); break;
        case 9: state.pressed_buttons.erase (button::start); break;
        case 10: state.pressed_buttons.erase (button::left_thumb); break;
        case 11: state.pressed_buttons.erase (button::right_thumb); break;
        default: break;
    };
}

void iokit_gamepad::on_device_axis_moved (Device* device, int axis_id, float value, float event_value) {

    switch (axis_id) {
        case 0: case 1: case 2: case 3:
        {
            const float THUMBSTICK_DEADZONE = 0.07f;
            if (value < THUMBSTICK_DEADZONE && value > -THUMBSTICK_DEADZONE) { value = 0.0f; }

            if (axis_id == 0)      state.left_stick.x = value;
            else if (axis_id == 1) state.left_stick.y = -value;
            else if (axis_id == 2) state.right_stick.x = value;
            else if (axis_id == 3) state.right_stick.y = -value;
            break;
        }
        case 4:
        {
            if (sge::math::is_zero (0.0f - event_value)) {
                state.pressed_buttons.insert (button::dpad_up);
            }
            else if (sge::math::is_zero (1.0f - event_value)) {
                state.pressed_buttons.insert (button::dpad_up);
                state.pressed_buttons.insert (button::dpad_right);
            }
            else if (sge::math::is_zero (2.0f - event_value)) {
                state.pressed_buttons.insert (button::dpad_right);
            }
            else if (sge::math::is_zero (3.0f - event_value)) {
                state.pressed_buttons.insert (button::dpad_right);
                state.pressed_buttons.insert (button::dpad_down);
            }
            else if (sge::math::is_zero (4.0f - event_value)) {
                state.pressed_buttons.insert (button::dpad_down);
            }
            else if (sge::math::is_zero (5.0f - event_value)) {
                state.pressed_buttons.insert (button::dpad_left);
                state.pressed_buttons.insert (button::dpad_down);
            }
            else if (sge::math::is_zero (6.0f - event_value)) {
                state.pressed_buttons.insert (button::dpad_left);
            }
            else if (sge::math::is_zero (7.0f - event_value)) {
                state.pressed_buttons.insert (button::dpad_left);
                state.pressed_buttons.insert (button::dpad_up);
            }
            else if (sge::math::is_zero (8.0f - event_value)) {
                state.pressed_buttons.erase (button::dpad_up);
                state.pressed_buttons.erase (button::dpad_right);
                state.pressed_buttons.erase (button::dpad_down);
                state.pressed_buttons.erase (button::dpad_left);
            }

            break;
        }
        case 5: case 6:
        {
            const float TRIGGER_DEADZONE = 0.01f;
            if (value < -1.0f + TRIGGER_DEADZONE) { value = -1.0f; }
            
            if (axis_id == 5)      state.left_trigger = (1.0f + value) / 2.0f;
            else if (axis_id == 6) state.right_trigger = (1.0f + value) / 2.0f;
            break;
        }
        default:
            break;
    }
}

#endif

