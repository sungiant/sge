#pragma once

#include "sge.hh"


namespace sge::ext {

class keyboard : public runtime::view {

public:

    bool is_character_down          (wchar_t z)                         const { return characters_current.find (z) != characters_current.end (); }
    bool is_character_up            (wchar_t z)                         const { return characters_current.find (z) == characters_current.end (); }
    bool was_character_down         (wchar_t z)                         const { return characters_previous.find (z) != characters_previous.end (); }
    bool was_character_up           (wchar_t z)                         const { return characters_previous.find (z) == characters_previous.end (); }
    bool character_just_pressed     (wchar_t z)                         const { return is_character_down (z) && was_character_up (z); }
    bool character_just_released    (wchar_t z)                         const { return is_character_up (z) && was_character_down (z); }

    bool is_key_down                (runtime::keyboard_key z)           const { return keys_current.find (z) != keys_current.end (); }
    bool is_key_up                  (runtime::keyboard_key z)           const { return keys_current.find (z) == keys_current.end (); }
    bool was_key_down               (runtime::keyboard_key z)           const { return keys_previous.find (z) != keys_previous.end (); }
    bool was_key_up                 (runtime::keyboard_key z)           const { return keys_previous.find (z) == keys_previous.end (); }
    bool key_just_pressed           (runtime::keyboard_key z)           const { return is_key_down (z) && was_key_up (z); }
    bool key_just_released          (runtime::keyboard_key z)           const { return is_key_up (z) && was_key_down (z); }

    bool is_lock_locked             (runtime::keyboard_lock z)          const { return locked_locks.find (z) != locked_locks.end (); }
    bool is_lock_down               (runtime::keyboard_lock z)          const { return pressed_locks.find (z) != pressed_locks.end (); }
        

private:
    std::unordered_set<runtime::keyboard_key> keys_current;
    std::unordered_set<runtime::keyboard_key> keys_previous;
    
    std::unordered_set<wchar_t> characters_current;
    std::unordered_set<wchar_t> characters_previous;
    
    std::unordered_set<runtime::keyboard_lock> pressed_locks;
    std::unordered_set<runtime::keyboard_lock> locked_locks;

public:
    
    keyboard (const runtime::api& z) : runtime::view (z) {
        keys_current.reserve((size_t) runtime::keyboard_key::COUNT);
        keys_previous.reserve((size_t) runtime::keyboard_key::COUNT);
    }

    virtual void update () override {
        uint32_t sz = 0;
        
        { // keys
            static std::array<runtime::keyboard_key, (size_t) runtime::keyboard_key::COUNT> keys_arr;
            
            keys_previous = keys_current;
            keys_current.clear();
            
            sge.input__keyboard_pressed_keys (&sz, nullptr);
            assert (sz <= keys_arr.size ());
            sge.input__keyboard_pressed_keys (&sz, keys_arr.data());
            
            for (uint32_t i = 0; i < sz; ++i)
                keys_current.insert(keys_arr[i]);
        }
        
        { // characters
            static std::array<wchar_t, (size_t) runtime::keyboard_character::COUNT> chars_arr;
            
            characters_previous = characters_current;
            characters_current.clear();
            
            sge.input__keyboard_pressed_characters (&sz, nullptr);
            assert (sz <= chars_arr.size ());
            sge.input__keyboard_pressed_characters (&sz, chars_arr.data());
            
            for (uint32_t i = 0; i < sz; ++i)
                characters_current.insert(chars_arr[i]);
        }
        
        { // locks
            static std::array<runtime::keyboard_lock, (size_t) runtime::keyboard_lock::COUNT> locks_arr;
            
            pressed_locks.clear ();
            sge.input__keyboard_pressed_locks (&sz, nullptr);
            assert (sz <= locks_arr.size ());
            sge.input__keyboard_pressed_locks (&sz, locks_arr.data());
            for (uint32_t i = 0; i < sz; ++i)
                pressed_locks.insert(locks_arr[i]);
            
            locked_locks.clear ();
            sge.input__keyboard_locked_locks (&sz, nullptr);
            assert (sz <= locks_arr.size ());
            sge.input__keyboard_locked_locks (&sz, locks_arr.data());
            for (uint32_t i = 0; i < sz; ++i)
                locked_locks.insert(locks_arr[i]);
        }
        
    }

    virtual void debug_ui () override {
        ImGui::Begin ("Keyboard");
        
        ImGui::Columns (3);
        
        int counter = 0;
        
        for (wchar_t wc : characters_current) {
            // todo: how to get imgui to show wchars
            ImGui::TextColored (ImGui::GetStyleColorVec4 (ImGuiCol_CheckMark), "character_%d: [%c]", counter++, wc);
            ImGui::NextColumn ();
        }
        
        for (int i = counter; i < (int) runtime::keyboard_character::COUNT; ++i) {
            ImGui::Text ("character_%d: ", counter++);
            ImGui::NextColumn ();
        }

        ImGui::Columns (1);
        
        ImGui::Separator ();
        ImGui::Columns (3);

        ImGui::Text ("caps_lk"); ImGui::NextColumn ();
        bool key_caps_lk_pressed = is_lock_down (runtime::keyboard_lock::caps_lk); ImGui::Checkbox ("pressed", &key_caps_lk_pressed); ImGui::NextColumn ();
        bool key_caps_lk_locked = is_lock_locked (runtime::keyboard_lock::caps_lk); ImGui::Checkbox ("locked", &key_caps_lk_locked); ImGui::NextColumn ();

        ImGui::Text ("scr_lk"); ImGui::NextColumn ();
        bool key_scr_lk_pressed = is_lock_down (runtime::keyboard_lock::scr_lk); ImGui::Checkbox ("pressed", &key_scr_lk_pressed); ImGui::NextColumn ();
        bool key_scr_lk_locked = is_lock_locked (runtime::keyboard_lock::scr_lk); ImGui::Checkbox ("locked", &key_scr_lk_locked); ImGui::NextColumn ();

        ImGui::Text ("num_lk"); ImGui::NextColumn ();
        bool key_num_lk_pressed = is_lock_down (runtime::keyboard_lock::num_lk); ImGui::Checkbox ("pressed", &key_num_lk_pressed); ImGui::NextColumn ();
        bool key_num_lk_locked = is_lock_locked (runtime::keyboard_lock::num_lk); ImGui::Checkbox ("locked", &key_num_lk_locked); ImGui::NextColumn ();

        ImGui::Columns (1);
        
        ImGui::Separator ();
         
        ImGui::Columns (3);

#define SGE_X(x) { bool key_ ## x = is_key_down (runtime::keyboard_key::x); ImGui::Checkbox (#x, &key_ ## x); ImGui::NextColumn (); }
        SGE_X (escape); SGE_X (enter); SGE_X (spacebar); SGE_X (shift); SGE_X (control); SGE_X (alt); SGE_X (backspace); SGE_X (tab);
        SGE_X (ins); SGE_X (del); SGE_X (home); SGE_X (end); SGE_X (page_up); SGE_X (page_down); SGE_X (right_click); SGE_X (prt_sc); SGE_X (pause);
        SGE_X (up); SGE_X (down); SGE_X (left); SGE_X (right);
        SGE_X (a); SGE_X (b); SGE_X (c); SGE_X (d); SGE_X (e); SGE_X (f); SGE_X (g); SGE_X (h); SGE_X (i); SGE_X (j); SGE_X (k); SGE_X (l); SGE_X (m);
        SGE_X (n); SGE_X (o); SGE_X (p); SGE_X (q); SGE_X (r); SGE_X (s); SGE_X (t); SGE_X (u); SGE_X (v); SGE_X (w); SGE_X (x); SGE_X (y); SGE_X (z);
        SGE_X (zero); SGE_X (one); SGE_X (two); SGE_X (three); SGE_X (four); SGE_X (five); SGE_X (six); SGE_X (seven); SGE_X (eight); SGE_X (nine);
        SGE_X (plus); SGE_X (minus); SGE_X (comma); SGE_X (period);
        SGE_X (windows); SGE_X (cmd);
        SGE_X (f1); SGE_X (f2); SGE_X (f3); SGE_X (f4); SGE_X (f5); SGE_X (f6); SGE_X (f7); SGE_X (f8); SGE_X (f9); SGE_X (f10); SGE_X (f11); SGE_X (f12);
        SGE_X (numpad_0); SGE_X (numpad_1); SGE_X (numpad_2); SGE_X (numpad_3); SGE_X (numpad_4); SGE_X (numpad_5); SGE_X (numpad_6); SGE_X (numpad_7); SGE_X (numpad_8); SGE_X (numpad_9);
        SGE_X (numpad_decimal); SGE_X (numpad_divide); SGE_X (numpad_multiply); SGE_X (numpad_subtract); SGE_X (numpad_add); SGE_X (numpad_enter); SGE_X (numpad_equals);
#undef SGE_X

        ImGui::Columns (1);
        
        ImGui::End();
    }
};

}


