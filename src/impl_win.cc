// SGE-WIN
// Reference SGE host implementation.
// ---------------------------------- //

#if TARGET_WIN32

#include <windows.h>

#include <cassert>
#include <cstdio>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <memory>
#include <variant>
#include <algorithm>

#include "sge_core.hh"

#include <windows.h>

// Windows Keyboard (stand alone class - independent of SGE)
// -------------------------------------------------------------------------- //
class win32_keyboard {
public:
    enum class key {
        escape, enter, spacebar, shift, control, alt, backspace, tab,
        ins, del, home, end, page_up, page_down, right_click, prt_sc, pause,
        up, down, left, right,
        a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
        zero, one, two, three, four, five, six, seven, eight, nine,
        plus, minus, comma, period,
        left_windows, right_windows,
        caps_lk, scr_lk, num_lk,
        f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
        numpad_0, numpad_1, numpad_2, numpad_3, numpad_4, numpad_5, numpad_6, numpad_7, numpad_8, numpad_9,
        numpad_decimal, numpad_divide, numpad_multiply, numpad_subtract, numpad_add, numpad_enter, numpad_equals,
    };

private:
    std::unordered_set<wchar_t> pressed_characters;
    std::unordered_set<key> pressed_keys;

    std::optional<DWORD> last_keydown;
    std::unordered_map<DWORD, wchar_t> active_translations;


public:
    win32_keyboard () = default;
    ~win32_keyboard () = default;

    std::unordered_set<wchar_t>& get_pressed_characters () { return pressed_characters; }
    std::unordered_set<key>& get_pressed_keys () { return pressed_keys; }

    bool is_char_pressed (wchar_t z) const { return pressed_characters.find (z) != pressed_characters.end (); }
    bool is_key_pressed (key z) const { return pressed_keys.find (z) != pressed_keys.end (); }

    bool is_caps_lk_locked () const { return GetKeyState (VK_CAPITAL) & 0x0001; }
    bool is_num_lk_locked () const { return GetKeyState (VK_NUMLOCK) & 0x0001; }
    bool is_scr_lk_locked () const { return GetKeyState (VK_SCROLL) & 0x0001; }

    void wnd_proc (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {

        WORD repeat_count = LOWORD(lparam);

        wchar_t msg[32];
        if (message == WM_CHAR && repeat_count <= 1) {
            assert (last_keydown.has_value ());

            wchar_t c = (wchar_t)wparam;
            if (last_keydown.has_value ()) {
                active_translations[last_keydown.value ()] = c;
            }
            pressed_characters.insert (c);

            swprintf_s (msg, L"insert wchar_t: %c\n", c);
            OutputDebugStringW (msg);

        }
        else if ((message == WM_KEYDOWN || message == WM_KEYUP) && repeat_count <= 1) {

            wchar_t c = (wchar_t)wparam;
            if (message == WM_KEYDOWN) {
                swprintf_s (msg, L"WM_KEYDOWN: 0x%x\n", c);
                OutputDebugStringW (msg);

                last_keydown = c;
            }
            else {
                swprintf_s (msg, L"WM_KEYUP: 0x%x\n", c);
                OutputDebugStringW (msg);

                if (active_translations.find (c) != active_translations.end ()) {
                    pressed_characters.erase (active_translations[c]);
                    active_translations.erase (c);

                    swprintf_s (msg, L"erase wchar_t: %c\n", active_translations[c]);
                    OutputDebugStringW (msg);
                }
                if (last_keydown.has_value () && last_keydown.value () == c)
                    last_keydown.reset ();
            }

            std::function<void (key)> f = [this, message](key k){
                if (message == WM_KEYDOWN) {
                    pressed_keys.insert (k);
                }
                else {
                    pressed_keys.erase (k);
                }
            };

            switch (wparam) { // Virtual-Key Codes

                case VK_ESCAPE:         f (key::escape);            break;
                case VK_RETURN:         f (key::enter);             break;
                case VK_SPACE:          f (key::spacebar);          break;
                case VK_SHIFT:          f (key::shift);             break;
                case VK_CONTROL:        f (key::control);           break;
                case VK_MENU:           f (key::alt);               break;
                case VK_BACK:           f (key::backspace);         break;
                case VK_TAB:            f (key::tab);               break;
                case VK_INSERT:         f (key::ins);               break;
                case VK_DELETE:         f (key::del);               break;
                case VK_HOME:           f (key::home);              break;
                case VK_END:            f (key::end);               break;
                case VK_PRIOR:          f (key::page_up);           break;
                case VK_NEXT:           f (key::page_down);         break;
                case VK_APPS:           f (key::right_click);       break;
                case VK_SNAPSHOT:       f (key::prt_sc);            break;
                case VK_PAUSE:          f (key::pause);             break;
                case VK_UP:             f (key::up);                break;
                case VK_DOWN:           f (key::down);              break;
                case VK_LEFT:           f (key::left);              break;
                case VK_RIGHT:          f (key::right);             break;
                case 0x41:              f (key::a);                 break;
                case 0x42:              f (key::b);                 break;
                case 0x43:              f (key::c);                 break;
                case 0x44:              f (key::d);                 break;
                case 0x45:              f (key::e);                 break;
                case 0x46:              f (key::f);                 break;
                case 0x47:              f (key::g);                 break;
                case 0x48:              f (key::h);                 break;
                case 0x49:              f (key::i);                 break;
                case 0x4A:              f (key::j);                 break;
                case 0x4B:              f (key::k);                 break;
                case 0x4C:              f (key::l);                 break;
                case 0x4D:              f (key::m);                 break;
                case 0x4E:              f (key::n);                 break;
                case 0x4F:              f (key::o);                 break;
                case 0x50:              f (key::p);                 break;
                case 0x51:              f (key::q);                 break;
                case 0x52:              f (key::r);                 break;
                case 0x53:              f (key::s);                 break;
                case 0x54:              f (key::t);                 break;
                case 0x55:              f (key::u);                 break;
                case 0x56:              f (key::v);                 break;
                case 0x57:              f (key::w);                 break;
                case 0x58:              f (key::x);                 break;
                case 0x59:              f (key::y);                 break;
                case 0x5A:              f (key::z);                 break;
                case 0x30:              f (key::zero);              break;
                case 0x31:              f (key::one);               break;
                case 0x32:              f (key::two);               break;
                case 0x33:              f (key::three);             break;
                case 0x34:              f (key::four);              break;
                case 0x35:              f (key::five);              break;
                case 0x36:              f (key::six);               break;
                case 0x37:              f (key::seven);             break;
                case 0x38:              f (key::eight);             break;
                case 0x39:              f (key::nine);              break;
                case VK_OEM_PLUS:       f (key::plus);              break;
                case VK_OEM_MINUS:      f (key::minus);             break;
                case VK_OEM_COMMA:      f (key::comma);             break;
                case VK_OEM_PERIOD:     f (key::period);            break;
                case VK_LWIN:           f (key::left_windows);      break;
                case VK_RWIN:           f (key::right_windows);     break;
                case VK_CAPITAL:        f (key::caps_lk);           break;
                case VK_SCROLL:         f (key::scr_lk);            break;
                case VK_NUMLOCK:        f (key::num_lk);            break;
                case VK_F1:             f (key::f1);                break;
                case VK_F2:             f (key::f2);                break;
                case VK_F3:             f (key::f3);                break;
                case VK_F4:             f (key::f4);                break;
                case VK_F5:             f (key::f5);                break;
                case VK_F6:             f (key::f6);                break;
                case VK_F7:             f (key::f7);                break;
                case VK_F8:             f (key::f8);                break;
                case VK_F9:             f (key::f9);                break;
                case VK_F10:            f (key::f10);               break;
                case VK_F11:            f (key::f11);               break;
                case VK_F12:            f (key::f12);               break;
                case VK_NUMPAD0:        f (key::numpad_0);          break;
                case VK_NUMPAD1:        f (key::numpad_1);          break;
                case VK_NUMPAD2:        f (key::numpad_2);          break;
                case VK_NUMPAD3:        f (key::numpad_3);          break;
                case VK_NUMPAD4:        f (key::numpad_4);          break;
                case VK_NUMPAD5:        f (key::numpad_5);          break;
                case VK_NUMPAD6:        f (key::numpad_6);          break;
                case VK_NUMPAD7:        f (key::numpad_7);          break;
                case VK_NUMPAD8:        f (key::numpad_8);          break;
                case VK_NUMPAD9:        f (key::numpad_9);          break;
                case VK_DECIMAL:        f (key::numpad_decimal);    break;
                case VK_DIVIDE:         f (key::numpad_divide);     break;
                case VK_MULTIPLY:       f (key::numpad_multiply);   break;
                case VK_SUBTRACT:       f (key::numpad_subtract);   break;
                case VK_ADD:            f (key::numpad_add);        break;
                default:                                            break;
            }
        }
        // not sure if these are needed...  just log them for now...
        else if (message == WM_SYSKEYDOWN) {
            swprintf_s (msg, L"WM_SYSKEYDOWN: 0x%x\n", (wchar_t)wparam);
            OutputDebugStringW (msg);
        }
        else if (message == WM_SYSCHAR) {
            swprintf_s (msg, L"WM_SYSCHAR: %c\n", (wchar_t)wparam);
            OutputDebugStringW (msg);
        }
        else if (message == WM_SYSKEYUP) {
            swprintf_s (msg, L"WM_SYSKEYUP: 0x%x\n", (wchar_t)wparam);
            OutputDebugStringW (msg);
        }
    }
};

// Windows Mouse (stand alone class - independent of SGE)
// -------------------------------------------------------------------------- //
class win32_mouse {
public:
    struct point { short x; short y; };
    enum class button { left, middle, right };

private:
    std::unordered_set<button> pressed_buttons;
    point position;
    int scrollwheel;

public:
    win32_mouse () : position ({ 0, 0 }) {}
    ~win32_mouse () = default;

    point get_position () const { return position; }
    int get_scrollwheel () const { return scrollwheel; }
    bool is_button_pressed (button z) const { return pressed_buttons.find (z) != pressed_buttons.end (); }

    void wnd_proc (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        switch (message) {
            case WM_LBUTTONDOWN:            pressed_buttons.insert (button::left);                      break;
            case WM_RBUTTONDOWN:            pressed_buttons.insert (button::right);                     break;
            case WM_MBUTTONDOWN:            pressed_buttons.insert (button::middle);                    break;
            case WM_LBUTTONUP:              pressed_buttons.erase  (button::left);                      break;
            case WM_RBUTTONUP:              pressed_buttons.erase  (button::right);                     break;
            case WM_MBUTTONUP:              pressed_buttons.erase  (button::middle);                    break;
            case WM_MOUSEWHEEL:             scrollwheel += GET_WHEEL_DELTA_WPARAM (wparam) / 120;       break;
            case WM_MOUSEMOVE:              position.x = LOWORD (lparam), position.y = HIWORD (lparam); break;
            default:                                                                                    break;
        }
    }

};


// XInput Gamepad (stand alone class - independent of SGE)
// -------------------------------------------------------------------------- //

#include <XInput.h>
#pragma comment(lib, "XInput.lib")

class xinput_gamepad
{
public:
    enum class index { one, two, three, four, MAX };
    struct vector { float x; float y; };
    enum class button { dpad_up, dpad_down, dpad_left, dpad_right, start, back, left_thumb, right_thumb, left_shoulder, right_shoulder, a, b, x, y };

private:

    std::array<XINPUT_STATE, static_cast<size_t>(index::MAX)>   xinput_states;                  // raw state from xinput
    std::array<int, static_cast<size_t>(index::MAX)>            xinput_return_codes;            // flag to indicate availablity
    int                                                         first_active_index = -1;        // first available controller

public:

    vector  get_left_stick      ()                      const { return get_left_stick ((index) first_active_index); }
    vector  get_right_stick     ()                      const { return get_right_stick ((index) first_active_index); }
    float   get_left_trigger    ()                      const { return get_left_trigger ((index) first_active_index); }
    float   get_right_trigger   ()                      const { return get_right_trigger ((index) first_active_index); }
    bool    is_button_pressed   (button z)              const { return is_button_pressed ((index) first_active_index, z); }

    vector  get_left_stick      (index i)               const { return vector { std::clamp ((float) xinput_states[(int) i].Gamepad.sThumbLX / (float)32767, -1.0f, 1.0f), std::clamp ((float) xinput_states[(int) i].Gamepad.sThumbLY / (float)32767, -1.0f, 1.0f) }; }
    vector  get_right_stick     (index i)               const { return vector { std::clamp ((float) xinput_states[(int) i].Gamepad.sThumbRX / (float)32767, -1.0f, 1.0f), std::clamp ((float) xinput_states[(int) i].Gamepad.sThumbRY / (float)32767, -1.0f, 1.0f) }; }
    float   get_left_trigger    (index i)               const { return std::clamp ((float) xinput_states[(int) i].Gamepad.bLeftTrigger / (float) 255, 0.0f, 1.0f); }
    float   get_right_trigger   (index i)               const { return std::clamp ((float) xinput_states[(int) i].Gamepad.bRightTrigger / (float) 255, 0.0f, 1.0f); }
    bool    is_button_pressed   (index i, button z)     const {
        auto& buttons = xinput_states[(int) i].Gamepad.wButtons;
        switch (z) {
            case button::dpad_up:           return buttons & XINPUT_GAMEPAD_DPAD_UP;
            case button::dpad_down:         return buttons & XINPUT_GAMEPAD_DPAD_DOWN;
            case button::dpad_left:         return buttons & XINPUT_GAMEPAD_DPAD_LEFT;
            case button::dpad_right:        return buttons & XINPUT_GAMEPAD_DPAD_RIGHT;
            case button::start:             return buttons & XINPUT_GAMEPAD_START;
            case button::back:              return buttons & XINPUT_GAMEPAD_BACK;
            case button::left_thumb:        return buttons & XINPUT_GAMEPAD_LEFT_THUMB;
            case button::right_thumb:       return buttons & XINPUT_GAMEPAD_RIGHT_THUMB;
            case button::left_shoulder:     return buttons & XINPUT_GAMEPAD_LEFT_SHOULDER;
            case button::right_shoulder:    return buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
            case button::a:                 return buttons & XINPUT_GAMEPAD_A;
            case button::b:                 return buttons & XINPUT_GAMEPAD_B;
            case button::x:                 return buttons & XINPUT_GAMEPAD_X;
            case button::y:                 return buttons & XINPUT_GAMEPAD_Y;
        }
        return false;
    }


    void vibrate (float left_motor, float right_motor) { vibrate ((index) first_active_index, left_motor, right_motor); }

    void vibrate (index i, float left_motor, float right_motor) {
        XINPUT_VIBRATION vibration;
        memset (&vibration, 0, sizeof (XINPUT_VIBRATION));
        vibration.wLeftMotorSpeed = (int)(left_motor * (float) 65535);
        vibration.wRightMotorSpeed = (int)(right_motor * (float) 65535);
        XInputSetState ((int) i, &vibration);
    }

    xinput_gamepad () {
        update_xinput_states ();
        for (int i = 0; i < static_cast<int>(index::MAX); ++i) {
            if (xinput_return_codes[i] == ERROR_SUCCESS)
                vibrate (static_cast<index>(i), 0.0f, 0.0f);
        }
    }

    ~xinput_gamepad () {
        for (int i = 0; i < static_cast<int>(index::MAX); ++i) {
            if (xinput_return_codes[i] == ERROR_SUCCESS)
                vibrate (static_cast<index>(i), 0.0f, 0.0f);
        }
    }

    void update () {
        update_xinput_states ();
        first_active_index = -1;
        for (int i = 0; i < static_cast<int>(index::MAX); ++i) {
            if (first_active_index == -1 && xinput_return_codes[i] == ERROR_SUCCESS) {
                first_active_index = i;
                break;
            }
        }
        if (first_active_index == -1)
            first_active_index = 0;
    }

private:

    void update_xinput_states () {
        for (int i = 0; i < static_cast<int>(index::MAX); ++i) {
            memset (&xinput_states[i], 0, sizeof (XINPUT_STATE));
            xinput_return_codes[i] = XInputGetState (i, &xinput_states[i]);
            // make adjustments to account for dead zones.
            if (xinput_states[i].Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && xinput_states[i].Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) xinput_states[i].Gamepad.sThumbLX = 0;
            if (xinput_states[i].Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && xinput_states[i].Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) xinput_states[i].Gamepad.sThumbLY = 0;
            if (xinput_states[i].Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && xinput_states[i].Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) xinput_states[i].Gamepad.sThumbRX = 0;
            if (xinput_states[i].Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && xinput_states[i].Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) xinput_states[i].Gamepad.sThumbRY = 0;
            if (xinput_states[i].Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) xinput_states[i].Gamepad.bLeftTrigger = 0;
            if (xinput_states[i].Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) xinput_states[i].Gamepad.bRightTrigger = 0;
        }
    }

};

// Windows utils
// -------------------------------------------------------------------------- //

bool enter_fullscreen (HWND hwnd, int fullscreen_width, int fullscreen_height, int colour_bits, int refresh_rate) {
    DEVMODE fullscreen_settings = {};
    EnumDisplaySettings (NULL, 0, &fullscreen_settings);
    fullscreen_settings.dmPelsWidth = fullscreen_width;
    fullscreen_settings.dmPelsHeight = fullscreen_height;
    fullscreen_settings.dmBitsPerPel = colour_bits;
    fullscreen_settings.dmDisplayFrequency = refresh_rate;
    fullscreen_settings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
    SetWindowLongPtr (hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
    SetWindowLongPtr (hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos (hwnd, HWND_TOPMOST, 0, 0, fullscreen_width, fullscreen_height, SWP_SHOWWINDOW);
    bool success = ChangeDisplaySettings (&fullscreen_settings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
    ShowWindow (hwnd, SW_MAXIMIZE);
    return success;
}

bool exit_fullscreen (HWND hwnd, int window_x, int window_y, int windowed_width, int windowed_height, bool maximised) {
    SetWindowLongPtr (hwnd, GWL_EXSTYLE, WS_EX_LEFT);
    SetWindowLongPtr (hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    bool success = ChangeDisplaySettings (NULL, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;
    SetWindowPos (hwnd, HWND_NOTOPMOST, window_x, window_y, windowed_width, windowed_height, SWP_SHOWWINDOW);
    ShowWindow (hwnd, maximised ? SW_MAXIMIZE : SW_RESTORE);
    return success;
}

int xpos (RECT r) { return r.left; }
int ypos (RECT r) { return r.top; }
int width (RECT r) { return r.right - r.left; }
int height (RECT r) { return r.bottom - r.top; }


// Windows SGE implementation
// -------------------------------------------------------------------------- //

// constant info
const char*                             g_default_title = "SGE";
int                                     g_screen_width; // can be scaled.
int                                     g_screen_height;
int                                     g_fullscreen_width; // not scaled, 1:1
int                                     g_fullscreen_height;
int                                     g_colour_bits;
int                                     g_refresh_rate;
int                                     g_window_style_x; // in normal windowed mode how much horizontal space does the window style take up
int                                     g_window_style_y; // in normal windowed mode how much vertical space does the window style take up

// input helpers
win32_keyboard                          g_keyboard;
win32_mouse                             g_mouse;
xinput_gamepad                          g_gamepad;


// current state
bool                                    g_fullscreen;              // is the window in full screen mode
bool                                    g_maximised;               // if (!fullscreen) { is the window in maximised mode } else { was the window in maximised mode }
bool                                    g_is_resizing;

RECT                                    g_latest_window_rect;               // the window rect at any time
RECT                                    g_latest_windowed_mode_window_rect; // the window rect whilst not maximised and not fullscreen

DWORD                                   g_style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER;

// the engine
std::unique_ptr<sge::core::engine>      g_sge;

void console (sge::app::configuration& configuration) {
    if (configuration.enable_console) {
        AllocConsole ();
        AttachConsole (GetCurrentProcessId ());
        freopen ("CON", "w", stdout);
        freopen ("CON", "w", stderr);
        SetConsoleTitle (g_default_title);
    }
    else {
        HWND hwnd = GetConsoleWindow ();
        ShowWindow (hwnd, SW_HIDE);
    }
}

void calculate_sge_container_state (sge::core::container_state& container) {
    container.is_resizing = g_is_resizing;

    if (g_fullscreen) {
        container.current_width = /* g_fullscreen_width */ width (g_latest_window_rect);
        container.current_height = /* g_fullscreen_height */ height (g_latest_window_rect);
    }
    else {
        container.current_width = width (g_latest_window_rect) - g_window_style_x;
        container.current_height = height (g_latest_window_rect) - g_window_style_y;
    }
}

void calculate_sge_input_state (sge::input_state& input) {
    // keyboard
    int keyboard_i = 0;
    auto keyboard_characters = g_keyboard.get_pressed_characters ();
    assert (keyboard_characters.size () <= 10);
    for (wchar_t character : keyboard_characters) {
        int i = static_cast<int> (sge::input_control_identifier::kc_0) + keyboard_i;
        auto id = static_cast<sge::input_control_identifier> (i);
        input[id] = static_cast<wchar_t> (character);
        ++keyboard_i;
    }

#define SGEX1(x) { if (g_keyboard.is_key_pressed (win32_keyboard::key::x)) input[sge::input_control_identifier::kb_ ## x] = true; }
#define SGEX2(x, y) { if (g_keyboard.is_key_pressed (win32_keyboard::key::x)) input[sge::input_control_identifier::kb_ ## y] = true; }

    SGEX1(escape); SGEX1(enter); SGEX1(spacebar); SGEX1(shift); SGEX1(control); SGEX1(alt); SGEX1(backspace); SGEX1(tab);
    SGEX1(ins); SGEX1(del); SGEX1(home); SGEX1(end); SGEX1(page_up); SGEX1(page_down); SGEX1(right_click); SGEX1(prt_sc); SGEX1(pause); SGEX1(up); SGEX1(down); SGEX1(left); SGEX1(right);
    SGEX1(a); SGEX1(b); SGEX1(c); SGEX1(d); SGEX1(e); SGEX1(f); SGEX1(g); SGEX1(h); SGEX1(i); SGEX1(j); SGEX1(k); SGEX1(l); SGEX1(m);
    SGEX1(n); SGEX1(o); SGEX1(p); SGEX1(q); SGEX1(r); SGEX1(s); SGEX1(t); SGEX1(u); SGEX1(v); SGEX1(w); SGEX1(x); SGEX1(y); SGEX1(z);
    SGEX2(zero, 0); SGEX2(one, 1); SGEX2(two, 2); SGEX2(three, 3); SGEX2(four, 4); SGEX2(five, 5); SGEX2(six, 6); SGEX2(seven, 7); SGEX2(eight, 8); SGEX2(nine, 9);
    SGEX1(plus); SGEX1(minus); SGEX1(comma); SGEX1(period);
    SGEX2(left_windows, windows); SGEX2(right_windows, windows);
    SGEX1(f1); SGEX1(f2); SGEX1(f3); SGEX1(f4); SGEX1(f5); SGEX1(f6); SGEX1(f7); SGEX1(f8); SGEX1(f9); SGEX1(f10); SGEX1(f11); SGEX1(f12);
    SGEX1(numpad_0); SGEX1(numpad_1); SGEX1(numpad_2); SGEX1(numpad_3); SGEX1(numpad_4); SGEX1(numpad_5); SGEX1(numpad_6); SGEX1(numpad_7); SGEX1(numpad_8); SGEX1(numpad_9);
    SGEX1(numpad_decimal); SGEX1(numpad_divide); SGEX1(numpad_multiply); SGEX1(numpad_subtract); SGEX1(numpad_add); SGEX1(numpad_enter); SGEX1(numpad_equals);

#undef SGEX2
#undef SGEX1

    {
        const bool caps_lk_locked = g_keyboard.is_caps_lk_locked ();
        const bool caps_lk_pressed = g_keyboard.is_key_pressed (win32_keyboard::key::caps_lk);
        if (caps_lk_locked || caps_lk_pressed)
            input[sge::input_control_identifier::kq_caps_lk] = std::make_pair (caps_lk_locked, caps_lk_pressed);

        const bool scr_lk_locked = g_keyboard.is_scr_lk_locked ();
        const bool scr_lk_pressed = g_keyboard.is_key_pressed (win32_keyboard::key::scr_lk);
        if (scr_lk_locked || scr_lk_pressed)
            input[sge::input_control_identifier::kq_scr_lk] = std::make_pair (scr_lk_locked, scr_lk_pressed);

        const bool num_lk_locked = g_keyboard.is_num_lk_locked ();
        const bool num_lk_pressed = g_keyboard.is_key_pressed (win32_keyboard::key::num_lk);
        if (num_lk_locked || num_lk_pressed)
            input[sge::input_control_identifier::kq_num_lk] = std::make_pair (num_lk_locked, num_lk_pressed);
    }

    // mouse
    input[sge::input_control_identifier::md_scrollwheel] = g_mouse.get_scrollwheel ();
    auto mouse_position = g_mouse.get_position ();
    input[sge::input_control_identifier::mp_position] = sge::input_point_control { mouse_position.x, mouse_position.y };

#define SGE_X(x) { if (g_mouse.is_button_pressed (win32_mouse::button::x)) input[sge::input_control_identifier::mb_ ## x] = true; }

    SGE_X (left); SGE_X (middle); SGE_X (right);

#undef SGE_X

    // gamepad
    input[sge::input_control_identifier::ga_left_trigger] = g_gamepad.get_left_trigger ();
    input[sge::input_control_identifier::ga_right_trigger] = g_gamepad.get_right_trigger ();
    auto gamepad_left_stick = g_gamepad.get_left_stick ();
    input[sge::input_control_identifier::ga_left_stick_x] = gamepad_left_stick.x;
    input[sge::input_control_identifier::ga_left_stick_y] = gamepad_left_stick.y;
    auto gamepad_right_stick = g_gamepad.get_right_stick ();
    input[sge::input_control_identifier::ga_right_stick_x] = gamepad_right_stick.x;
    input[sge::input_control_identifier::ga_right_stick_y] = gamepad_right_stick.y;

#define SGE_X(x) { if (g_gamepad.is_button_pressed (xinput_gamepad::button::x)) input[sge::input_control_identifier::gb_ ## x] = true; }

    SGE_X (dpad_up); SGE_X (dpad_down); SGE_X (dpad_left); SGE_X (dpad_right);
    SGE_X (start); SGE_X (back);
    SGE_X (left_thumb); SGE_X (right_thumb); SGE_X (left_shoulder); SGE_X (right_shoulder);
    SGE_X (a); SGE_X (b); SGE_X (x); SGE_X (y);

#undef SGE_X
}

void toggle_fullscreen (HWND hwnd) {

    g_fullscreen = !g_fullscreen;

    if (g_fullscreen) {
        //GetWindowRect (hwnd, &g_window_rect);
        enter_fullscreen (hwnd, g_fullscreen_width, g_fullscreen_height, g_colour_bits, g_refresh_rate);
        g_is_resizing = true;
    }
    else {
        exit_fullscreen (hwnd, xpos (g_latest_windowed_mode_window_rect), ypos(g_latest_windowed_mode_window_rect), width (g_latest_windowed_mode_window_rect), height (g_latest_windowed_mode_window_rect), g_maximised);
        g_is_resizing = true;
    }
}

void set_window_size (HWND hwnd, int w, int h) {
    const int center_x = (width (g_latest_windowed_mode_window_rect) / 2) + xpos (g_latest_windowed_mode_window_rect);
    const int center_y = (height (g_latest_windowed_mode_window_rect) / 2) + ypos (g_latest_windowed_mode_window_rect);
    const int target_window_x = center_x - (w / 2);
    const int target_window_y = center_y - (h / 2);

    RECT r;
    r.left = target_window_x;
    r.top = target_window_y;
    r.right = target_window_x + w;
    r.bottom = target_window_y + h;

    AdjustWindowRect (&r, g_style, FALSE);

    if (r.left < 0) { int delta = abs (r.left); r.left += delta; r.right += delta; }
    if (r.top < 0) { int delta = abs (r.top); r.top += delta; r.bottom += delta; }
    if (r.right > g_screen_width) { int delta = r.right - g_screen_width; r.left -= delta; r.right -= delta; }
    if (r.bottom > g_screen_height) { int delta = r.bottom - g_screen_height; r.top -= delta; r.bottom -= delta; }

    g_latest_windowed_mode_window_rect = r;

    if (g_fullscreen) { /* do nothing */ }
    else if (g_maximised) {
        SetWindowPos (hwnd, HWND_NOTOPMOST, xpos (r), ypos (r), width (r), height (r), SWP_SHOWWINDOW);
        g_maximised = false;
        ShowWindow (hwnd, SW_RESTORE);
    }
    else {
        SetWindowPos (hwnd, HWND_NOTOPMOST, xpos (r), ypos (r), width (r), height (r), SWP_SHOWWINDOW);
    }
}

int run (HINSTANCE hinst, HWND hwnd, sge::app::configuration& configuration) {


    g_sge->setup (hinst, hwnd);

    g_sge->register_set_window_title_callback ([hwnd](const char* s) { SetWindowText (hwnd, s);  });
    g_sge->register_set_window_fullscreen_callback ([hwnd](bool v) { if (v != g_fullscreen) toggle_fullscreen (hwnd); });
    g_sge->register_set_window_size_callback ([hwnd](int w, int h) { set_window_size (hwnd, w, h); });
    g_sge->register_shutdown_request_callback ([hwnd]() { PostQuitMessage (0); });

    g_sge->start ();

    // update loop (as fast as possible)
    int return_code = 0;
    MSG message = {};
    while (return_code == 0) {

        // reset flags
        g_is_resizing = false;

        // deal will all windows messages
        while (true) {
            if (PeekMessage (&message, NULL, 0, 0, PM_REMOVE)) {
                if (WM_QUIT == message.message) {
                    return_code = 1;
                    break;
                }
                else {
                    TranslateMessage (&message);
                    DispatchMessage (&message);
                }
            }
            else break; // done processing windows messages
        }

        if (return_code != 0)
            break;

        // update application side stuff
        g_gamepad.update ();

        // update the engine
        sge::core::container_state container_state;
        sge::input_state input_state;

        calculate_sge_container_state (container_state);
        calculate_sge_input_state (input_state);

        g_sge->update (container_state, input_state);

    }

    g_sge->stop ();
    g_sge->shutdown ();

    return return_code;
}


LRESULT wnd_proc (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_QUIT:  {
            //printf ("quitting\n");
        } break;

        case WM_ENTERSIZEMOVE:
        case WM_EXITSIZEMOVE: {
            g_is_resizing = (message == WM_ENTERSIZEMOVE);
            GetWindowRect (hwnd, &g_latest_window_rect);
            g_latest_windowed_mode_window_rect = g_latest_window_rect;
        } break;

        case WM_SIZE: {
            if (wparam == SIZE_MAXIMIZED) {
                if (!g_fullscreen) {
                    g_maximised = true;
                }
                GetWindowRect (hwnd, &g_latest_window_rect);
            }
            if (wparam == SIZE_RESTORED) {
                if (!g_fullscreen) {
                    g_maximised = false;
                }
                GetWindowRect (hwnd, &g_latest_window_rect);
            }
        } break;

        case WM_DESTROY: {
            DestroyWindow (hwnd);
            PostQuitMessage (0);
        } break;

        case WM_PAINT: {
            ValidateRect (hwnd, NULL);
        } break;
    }

    g_keyboard.wnd_proc (hwnd, message, wparam, lparam);
    g_mouse.wnd_proc (hwnd, message, wparam, lparam);

    return DefWindowProc (hwnd, message, wparam, lparam);
}

int CALLBACK WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR lpcmdln, int ncmdshow) {
    g_sge = std::make_unique<sge::core::engine>();
    auto configuration = sge::app::get_configuration ();
    console (configuration);
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof (WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wnd_proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hinst;
    wcex.hIcon = LoadIcon (hinst, MAKEINTRESOURCE (IDI_APPLICATION));
    wcex.hCursor = LoadCursor (NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = configuration.app_name.c_str ();
    wcex.hIconSm = LoadIcon (wcex.hInstance, MAKEINTRESOURCE (IDI_APPLICATION));
    assert (RegisterClassEx (&wcex));

    if (configuration.ignore_os_dpi_scaling) {
        SetProcessDPIAware (); // need to do this before calling AdjustWindowRect
    }

    g_screen_width = GetSystemMetrics (SM_CXSCREEN);
    g_screen_height = GetSystemMetrics (SM_CYSCREEN);
    const int target_window_width = configuration.app_width;
    const int target_window_height = configuration.app_height;
    const int target_window_x = g_screen_width / 2 - target_window_width / 2;
    const int target_window_y = g_screen_height / 2 - target_window_height / 2;
    g_latest_window_rect.left = target_window_x;
    g_latest_window_rect.top = target_window_y;
    g_latest_window_rect.right = target_window_x + target_window_width;
    g_latest_window_rect.bottom = target_window_y + target_window_height;
    AdjustWindowRect (&g_latest_window_rect, g_style, FALSE);
    g_window_style_x = width(g_latest_window_rect) - configuration.app_width;
    g_window_style_y = height(g_latest_window_rect) - configuration.app_height;
    g_latest_windowed_mode_window_rect = g_latest_window_rect;

    HWND hwnd = CreateWindow (
        configuration.app_name.c_str (),
        configuration.app_name.c_str (),
        g_style,
        xpos (g_latest_window_rect),
        ypos (g_latest_window_rect),
        width (g_latest_window_rect),
        height (g_latest_window_rect),
        NULL,
        NULL,
        hinst,
        NULL);
    assert (hwnd);
    ShowWindow (hwnd, SW_SHOW);
    SetForegroundWindow (hwnd);
    SetFocus (hwnd);

    HDC windowHDC = GetDC (hwnd);
    g_fullscreen_width = GetDeviceCaps (windowHDC, DESKTOPHORZRES);
    g_fullscreen_height = GetDeviceCaps (windowHDC, DESKTOPVERTRES);
    g_colour_bits = GetDeviceCaps (windowHDC, BITSPIXEL);
    g_refresh_rate = GetDeviceCaps (windowHDC, VREFRESH);

    int result = run (hinst, hwnd, configuration);

    assert (g_sge.get ());
    g_sge.reset ();

    return result;
}

#endif
