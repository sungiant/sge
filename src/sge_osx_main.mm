#if TARGET_MACOSX

#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <unordered_set>
#include <algorithm>
#include <optional>

#include <Cocoa/Cocoa.h>
#include <MetalKit/MTKView.h>

#include "sge_core.hpp"
#include "sge_app.hpp"

#include "sge_osx_gamepad.hpp"

#define SGE_OSX_INPUT_DEBUG 0

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
    
    
#define SGE_OSX_TRANSLATE_KEY_PRESS_EX(x, y) { if (g_keyboard_pressed_fns.find (x) != g_keyboard_pressed_fns.end()) input[sge::input_control_identifier::kb_ ## y] = true; }

    SGE_OSX_TRANSLATE_KEY_PRESS_EX (27, escape);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (13, enter);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (32, spacebar);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSEventModifierFlagShift, shift);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSEventModifierFlagControl, control);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSEventModifierFlagOption, alt);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (127, backspace);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (9, tab);
    
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSInsertFunctionKey, ins);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSDeleteFunctionKey, del);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSHomeFunctionKey, home);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSEndFunctionKey, end);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSPageUpFunctionKey, page_up);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSPageDownFunctionKey, page_down);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSMenuFunctionKey, right_click);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSPrintScreenFunctionKey, prt_sc);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSPauseFunctionKey, pause);
    
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSUpArrowFunctionKey, up);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSDownArrowFunctionKey, down);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSLeftArrowFunctionKey, left);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSRightArrowFunctionKey, right);
    
#define SGE_OSX_CHARACTER_KEY_PRESS_EX(x, y) { case x: input[sge::input_control_identifier::kb_ ## y] = true; break; }

    for (wchar_t character : g_keyboard_pressed_characters) {
        switch (toupper (character)) {
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('A', a);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('B', b);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('C', c);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('D', d);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('E', e);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('F', f);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('G', g);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('H', h);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('I', i);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('J', j);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('K', k);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('L', l);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('M', m);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('N', n);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('O', o);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('P', p);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('Q', q);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('R', r);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('S', s);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('T', t);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('U', u);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('V', v);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('W', w);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('X', x);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('Y', y);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('Z', z);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('0', 0);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('1', 1);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('2', 2);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('3', 3);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('4', 4);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('5', 5);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('6', 6);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('7', 7);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('8', 8);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('9', 9);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('+', plus);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('-', minus);
            SGE_OSX_CHARACTER_KEY_PRESS_EX (',', comma);
            SGE_OSX_CHARACTER_KEY_PRESS_EX ('.', period);
            default: break;
        }
    }
   
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSEventModifierFlagCommand, cmd);
    
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF1FunctionKey, f1);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF2FunctionKey, f2);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF3FunctionKey, f3);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF4FunctionKey, f4);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF5FunctionKey, f5);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF6FunctionKey, f6);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF7FunctionKey, f7);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF8FunctionKey, f8);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF9FunctionKey, f9);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF10FunctionKey, f10);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF11FunctionKey, f11);
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (NSF12FunctionKey, f12);
    
    SGE_OSX_TRANSLATE_KEY_PRESS_EX (3, numpad_enter);
    
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
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::start)) input[sge::input_control_identifier::gb_start] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::back)) input[sge::input_control_identifier::gb_back] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::left_thumb)) input[sge::input_control_identifier::gb_left_thumb] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::right_thumb)) input[sge::input_control_identifier::gb_right_thumb] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::left_shoulder)) input[sge::input_control_identifier::gb_left_shoulder] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::right_shoulder)) input[sge::input_control_identifier::gb_right_shoulder] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::a)) input[sge::input_control_identifier::gb_a] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::b)) input[sge::input_control_identifier::gb_b] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::x)) input[sge::input_control_identifier::gb_x] = true;
    if (g_gamepad.is_button_pressed (iokit_gamepad::button::y)) input[sge::input_control_identifier::gb_y] = true;
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
