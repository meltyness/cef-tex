#include "Header.h"
#include <include/internal/cef_win.h>
#include <include/cef_app.h>

#include <OS.hpp>
#include <Image.hpp>
#include <ImageTexture.hpp>
#include <PoolArrays.hpp>
#include <Array.hpp>
#include <InputEvent.hpp>
#include <InputEventKey.hpp>

#include "key_mapping_windows.h"

using namespace godot;

void CEFTex::_register_methods() {
    register_method("_process", &CEFTex::_process);
    register_method("get_cef_rect", &CEFTex::get_cef_rect);                                                                                                                                                                                             
    register_method("set_cef_rect", &CEFTex::set_cef_rect);
    register_method("simulate_click", &CEFTex::simulate_click);
    register_method("simulate_motion", &CEFTex::simulate_motion);
    register_method("simulate_scroll", &CEFTex::simulate_scroll);
    register_method("simulate_key", &CEFTex::simulate_key);
    register_method("go_back", &CEFTex::go_back);
    register_method("grab_audio_samples", &CEFTex::grab_audio_samples);
}

CEFTex::CEFTex() {
    rect_width = 480;
    rect_height = 640;
    
    last_buffer.resize(rect_width * rect_height * 4);
    std::cout << "length of last buffer is: " << last_buffer.size() << std::endl;

    last_image.instance();
    last_image_texture.instance();
    this->audioHandler = new CTAudioHandler();
    this->renderHandler = new RenderHandler(rect_width, rect_height);
    this->loadHandler = new LoadHandler();
    this->browserClient = new BrowserClient(renderHandler, loadHandler, audioHandler);
    this->browserSettings.windowless_frame_rate = 30;
    this->window_info.SetAsWindowless(NULL);
}

CEFTex::~CEFTex() {
    // add your cleanup here
}

void CEFTex::_init() {
    // initialize any variables here
    time_passed = 0.0;
   
    CefMainArgs args;
    CefSettings settings;
    
    app_wrapper = new AppWrapper();

    

    // TODO: Temporary troubleshooting measure, revert to false
    settings.windowless_rendering_enabled = true;
    //settings.multi_threaded_message_loop = true;

    //std::cout << "" << OS::get_singleton()->get_data_dir();
    
    // TODO: Figure out how to fucking mke this a relative path
    godot::Godot::print_error("Set the path!", "OH WOW", "OMG", 24);
    CefString(&settings.browser_subprocess_path).FromString("C:\\Users\\thomas\\OneDrive\\Code and Projects\\GammaSP\\godot_workspace\\cef-project-new\\third_party\\cef\\cef_binary_106.0.26+ge105400+chromium-106.0.5249.91_windows64\\tests\\cefclient\\Release\\cefclient.exe");
    godot::Godot::print_error("Cefinishulize", "OH WOW", "OMG", 24);
    CefInitialize(args, settings, app_wrapper, nullptr);
    godot::Godot::print_error("Axing Proxcess", "OH WOW", "OMG", 24);
    int exec_result = CefExecuteProcess(args, app_wrapper, nullptr);
    godot::Godot::print_error("Creat browse sync", "OH WOW", "OMG", 24);

    browser = CefBrowserHost::CreateBrowserSync(window_info, 
                                                browserClient.get(), 
                                                "chrome://version", 
                                                browserSettings, 
                                                nullptr, nullptr);
    godot::Godot::print_error("Created browser", "OH WOW", "OMG", 24);
    //browser->GetHost()->SetAudioMuted(true);
    godot::Godot::print_error("post - chrome", "OH WOW", "OMG", 24);
}

void CEFTex::_process(float delta) {
    CefDoMessageLoopWork();
    godot::Godot::print_error("Did message work, wow!", "OH WOW", "OMG", 24);
    if (this->loadHandler->success) {
        godot::Godot::print_error("Page fucking loaded, holy fuck!", "OH WOW", "OMG", 24);
        memcpy((void*)(last_buffer.write().ptr()), renderHandler->texture_data, rect_width * rect_height * 4);
        last_image->create_from_data(rect_width, rect_height, false, godot::Image::Format::FORMAT_RGBA8, last_buffer);
        last_image_texture->create_from_image(last_image, godot::Texture::FLAGS_DEFAULT);
        this->set_texture(last_image_texture);
    }
}

void godot::CEFTex::get_cef_rect(int width, int height)
{
}

void godot::CEFTex::set_cef_rect(float width, float height)
{
    std::cout << "Initial Size: (" << this->rect_width << ", " << this->rect_height << ")" << std::endl;
    this->rect_height *= height;
    this->rect_width *= width;

    last_buffer.resize(this->rect_width * this->rect_height * 4);
    this->renderHandler->resize(this->rect_width, this->rect_height);

    std::cout << "New Requested Size: (" << this->rect_width << ", " << this->rect_height << ")" << std::endl;
    this->browser->GetHost()->NotifyScreenInfoChanged();
    this->browser->GetHost()->WasResized();
    
}

void godot::CEFTex::simulate_click(int x, int y, bool pressed)
{
    CefMouseEvent c;
    CefBrowserHost::MouseButtonType b;
    c.x = x;
    c.y = y;
    
    b = MBT_LEFT;
    browser->GetHost()->SendMouseClickEvent(c, b, !pressed, 1);
}

void godot::CEFTex::simulate_motion(int x, int y, int dx, int dy, int button_flags)
{
    CefMouseEvent c;
    
    c.x = x;
    c.y = y;

    CefBrowserHost::MouseButtonType b;

    if (button_flags != 0) {
        b = MBT_LEFT;
    }

    browser->GetHost()->SendMouseMoveEvent(c, false);
}

void godot::CEFTex::simulate_scroll(int x, int y, int dx, int dy)
{
    CefMouseEvent c;

    c.x = x;
    c.y = y;
    std::cout << "scroll signal received with " << dx << ", " << dy << std::endl;
    browser->GetHost()->SendMouseWheelEvent(c, dx, dy);
}

void godot::CEFTex::go_back() {
    browser->GetHost()->GetBrowser()->GoBack();
}

PoolVector2Array godot::CEFTex::grab_audio_samples(int num_samples) {
    PoolVector2Array samples;
    // TODO: Channel settings
    int channels = 2;
    float** temp_audio_buffer = (float**)malloc(channels * sizeof(float*));

    for (int i = 0; i < channels; i++) {
        temp_audio_buffer[i] = (float*) malloc(num_samples * sizeof(float));
    }
    this->audioHandler->GrabPacket(temp_audio_buffer, num_samples);

    for (int j = 0; j < num_samples; j++) {
        samples.append(Vector2(temp_audio_buffer[0][j], temp_audio_buffer[1][j]));        
    }

    return samples;
}

void godot::CEFTex::simulate_key(int scancode_in, bool pressed) {
    std::cout << "Original scancode: " << scancode_in << " ";
    Key k = KeyMappingWindows::get_keysym(scancode_in);

    int message;
    if (pressed) {
        message = WM_KEYDOWN;
    }
    else {
        message = WM_KEYUP;
    }
    CefKeyEvent event;
    event.windows_key_code = (int)k;
    event.native_key_code = (int)k;
    event.is_system_key = 0;

    if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
        event.type = KEYEVENT_RAWKEYDOWN;
    else if (message == WM_KEYUP || message == WM_SYSKEYUP)
        event.type = KEYEVENT_KEYUP;
    else
        event.type = KEYEVENT_CHAR;
    event.modifiers = this->GetCefKeyboardModifiers((WPARAM)(int)k, (LPARAM)(int)k);


    int key_code = (int)k;
    std::cout << "key pressed: " << (int)k;

    event.is_system_key = false;

    BYTE VkCode = LOBYTE(VkKeyScanA(key_code));
    std::cout << " translated to: " << (int)VkCode;
    UINT scanCode = MapVirtualKey(VkCode, MAPVK_VK_TO_VSC);
    std::cout << " and listed scan code: " << (int)scanCode;
    event.native_key_code = (scanCode << 16) | 1;  // key scan code + // key repeat count
    event.windows_key_code = VkCode;

    if (pressed) {
        event.type = KEYEVENT_RAWKEYDOWN;
        browser->GetHost()->SendKeyEvent(event);

        event.windows_key_code = key_code;
        event.type = KEYEVENT_CHAR;
        if (event.modifiers == 0) {
            browser->GetHost()->SendKeyEvent(event);
        }
    } else {
        event.windows_key_code = VkCode;
        // bits 30 and 31 should be always 1 for WM_KEYUP
        event.native_key_code |= 0xC0000000;
        event.type = KEYEVENT_KEYUP;
        browser->GetHost()->SendKeyEvent(event);
    }
}

int godot::CEFTex::GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) {
    int modifiers = 0;
    switch (wparam) {
    case VK_RETURN:
        if ((lparam >> 16) & KF_EXTENDED)
            modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
        if (!((lparam >> 16) & KF_EXTENDED))
            modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
    case VK_NUMLOCK:
    case VK_NUMPAD0:
    case VK_NUMPAD1:
    case VK_NUMPAD2:
    case VK_NUMPAD3:
    case VK_NUMPAD4:
    case VK_NUMPAD5:
    case VK_NUMPAD6:
    case VK_NUMPAD7:
    case VK_NUMPAD8:
    case VK_NUMPAD9:
    case VK_DIVIDE:
    case VK_MULTIPLY:
    case VK_SUBTRACT:
    case VK_ADD:
    case VK_DECIMAL:
    case VK_CLEAR:
        modifiers |= EVENTFLAG_IS_KEY_PAD;
        modifiers |= EVENTFLAG_IS_LEFT;
        break;
    case VK_SHIFT:
        modifiers |= EVENTFLAG_SHIFT_DOWN;
        modifiers |= EVENTFLAG_IS_LEFT;
        break;
    case VK_CONTROL:
        modifiers |= EVENTFLAG_CONTROL_DOWN;
        modifiers |= EVENTFLAG_IS_LEFT;
        break;
    case VK_MENU:
        modifiers |= EVENTFLAG_ALT_DOWN;
        break;
    case VK_LWIN:
        modifiers |= EVENTFLAG_IS_LEFT;
        break;
    case VK_RWIN:
        modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    }
    return modifiers;
}