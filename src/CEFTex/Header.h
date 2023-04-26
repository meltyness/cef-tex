#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#include <Godot.hpp>
#include <OS.hpp>
#include <ImageTexture.hpp>
#include <TextureRect.hpp>
#include <core/GodotGlobal.hpp>

#include "cef_handler.cpp"

namespace godot {

    class CEFTex : public TextureRect {
        GODOT_CLASS(CEFTex, TextureRect)

    private:
        float time_passed;
        int rect_width;
        int rect_height;
        RenderHandler *renderHandler;
        CefRefPtr<LoadHandler> loadHandler;
        CefRefPtr<CefBrowser> browser;
        CefRefPtr<BrowserClient> browserClient;
        CefRefPtr<CTAudioHandler> audioHandler;
        CefRefPtr<AppWrapper> app_wrapper;
        CefWindowInfo window_info;
        CefBrowserSettings browserSettings;
        Ref<ImageTexture> last_image_texture;
        Ref<Image> last_image;
        PoolByteArray last_buffer;

    public:
        static void _register_methods();

        CEFTex();
        ~CEFTex();

        void _init(); // our initializer called by Godot
        void _process(float delta);
        void get_cef_rect(int width, int height);
        void set_cef_rect(float width, float height);
        void simulate_click(int x, int y, bool pressed);
        void simulate_motion(int x, int y, int dx, int dy, int button_flags);
        void simulate_scroll(int x, int y, int dx, int dy);
        void go_back();
        PoolVector2Array grab_audio_samples(int num_samples);
        void simulate_key(int scancode_in, bool pressed);
        int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam);
    };

}

#endif#pragma once
