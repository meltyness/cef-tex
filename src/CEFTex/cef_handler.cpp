#include <stdio.h>
#include <iostream>
#include <sstream>

#include <include/cef_app.h>
#include <include/cef_base.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include <include/cef_life_span_handler.h>
#include <include/cef_load_handler.h>
#include <include/wrapper/cef_helpers.h>
#include "include/base/cef_build.h"
#include "include/base/cef_ref_counted.h"

#ifndef CEF_H
#define CEF_H

class CTAudioHandler :
    public CefAudioHandler
{
    public:
        CefAudioParameters* last_params = nullptr;

        // A ring buffer for each channel
        int requested_fpb = 4096;
        int packet_rings = 16;
        int num_channels;
        int load_position;
        int play_position;
        bool audio_ready = false;
        float** audio_buffers;

        CTAudioHandler() {
            // We can receive 6 packets total.            

            load_position = 0;
            play_position = 0;

            num_channels = 2;
            audio_buffers = (float**)malloc(sizeof(float*) * num_channels);
            for (int i = 0; i < num_channels; i++) {
                audio_buffers[i] = (float*) malloc(packet_rings * requested_fpb * sizeof(float));
                for (int j = 0; j < packet_rings * requested_fpb; j++) {
                    audio_buffers[i][j] = 0.0;
                }
            }
        }

        ~CTAudioHandler() {

        }

        ///
        // Called on the UI thread to allow configuration of audio stream parameters.
        // Return true to proceed with audio stream capture, or false to cancel it.
        // All members of |params| can optionally be configured here, but they are
        // also pre-filled with some sensible defaults.
        ///
        /*--cef()--*/
        bool GetAudioParameters(CefRefPtr<CefBrowser> browser,
            CefAudioParameters& params) {
            std::cout << "GetAudioParameters() called" << std::endl;
            last_params = new CefAudioParameters();
            last_params->sample_rate = 22050;
            last_params->channel_layout = CEF_CHANNEL_LAYOUT_STEREO;
            last_params->frames_per_buffer = requested_fpb;
            
            params = CefAudioParameters(*last_params);

            return true;
        }

        ///
        // Called on a browser audio capture thread when the browser starts
        // streaming audio. OnAudioStreamStopped will always be called after
        // OnAudioStreamStarted; both methods may be called multiple times
        // for the same browser. |params| contains the audio parameters like
        // sample rate and channel layout. |channels| is the number of channels.
        ///
        /*--cef()--*/
        void OnAudioStreamStarted(CefRefPtr<CefBrowser> browser,
            const CefAudioParameters& params,
            int channels) {

            delete last_params;
            last_params = new CefAudioParameters(params);
            requested_fpb = last_params->frames_per_buffer;

            if (last_params->channel_layout != CEF_CHANNEL_LAYOUT_STEREO) {
                // TODO: There's nothing you can do
                std::cout << "panik! Illogical, channel layout bad!" << std::endl;
            }

            for (int i = 0; i < num_channels; i++) {
                free((void*)audio_buffers[i]);
                audio_buffers[i] = (float*)malloc(packet_rings * requested_fpb * sizeof(float));
                for (int j = 0; j < packet_rings * requested_fpb; j++) {
                    audio_buffers[i][j] = 0.0;
                }
            }

            audio_ready = true;
        }

        ///
        // Called on the audio stream thread when a PCM packet is received for the
        // stream. |data| is an array representing the raw PCM data as a floating
        // point type, i.e. 4-byte value(s). |frames| is the number of frames in the
        // PCM packet. |pts| is the presentation timestamp (in milliseconds since the
        // Unix Epoch) and represents the time at which the decompressed packet should
        // be presented to the user. Based on |frames| and the |channel_layout| value
        // passed to OnAudioStreamStarted you can calculate the size of the |data|
        // array in bytes.
        ///
        /*--cef()--*/
        void OnAudioStreamPacket(CefRefPtr<CefBrowser> browser,
            // TODO: Neon / SSE
            const float** data,
            int frames,
            int64 pts) {
            for (int i = 0; i < num_channels; i++) {
                for (int j = 0; j < frames; j++) {
                    audio_buffers[i][(j + load_position) % (packet_rings * requested_fpb)] = data[i][j];
                }
            }
            load_position = (load_position + frames) % (packet_rings * requested_fpb);
        }

        void GrabPacket(float** Dst, int requested_frames) {
            // TODO: Neon / SSE
            
            if (audio_ready) {
                int frames_available = 0;
                // Two cases: play_position/load_position straddles 0
                if (play_position > load_position) {
                    // straddle zero case
                    frames_available = ((packet_rings * requested_fpb) - play_position) + load_position;
                }
                else {
                    // vanilla case
                    frames_available = load_position - play_position;
                }

                if (frames_available > requested_frames) {
                    //std::cout << "Warning: Chromium not producing audio samples fast enough";
                    frames_available = requested_frames;
                }

                for (int i = 0; i < num_channels; i++) {
                    for (int j = 0; j < frames_available; j++) {
                        Dst[i][j] = audio_buffers[i][(j + play_position) % (packet_rings * requested_fpb)];
                    }
                }
                play_position = (play_position + frames_available) % (packet_rings * requested_fpb);
            }
            else {
                for (int i = 0; i < num_channels; i++) {
                    for (int j = 0; j < requested_frames; j++) {
                        Dst[i][j] = 0.0;
                    }
                }
            }
        }

        ///
        // Called on the UI thread when the stream has stopped. OnAudioSteamStopped
        // will always be called after OnAudioStreamStarted; both methods may be
        // called multiple times for the same stream.
        ///
        /*--cef()--*/
        void OnAudioStreamStopped(CefRefPtr<CefBrowser> browser) {
            play_position = 0;
            load_position = 0;

            std::cout << "OnAudioStreamStopped called";

            for (int i = 0; i < num_channels; i++) {
                for (int j = 0; j < packet_rings * requested_fpb; j++) {
                    audio_buffers[i][j] = 0.0;
                }
            }
            audio_ready = false;
        }

        ///
        // Called on the UI or audio stream thread when an error occurred. During the
        // stream creation phase this callback will be called on the UI thread while
        // in the capturing phase it will be called on the audio stream thread. The
        // stream will be stopped immediately.
        ///
        /*--cef()--*/
        void OnAudioStreamError(CefRefPtr<CefBrowser> browser,
            const CefString& message) {
        }
private:
    IMPLEMENT_REFCOUNTING(CTAudioHandler);
};

class AppWrapper :
    public CefApp
{
public:
    bool args_loaded = false;
    void OnBeforeCommandLineProcessing(const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) 
    {
        //std::cout << "void OnBeforeCommandLineProcessing\n";
        // Tweaking OSR performance by setting the same Chromium flags
        // as in upstream cefclient(Issue #240).

        // Headless per https://developers.google.com/web/updates/2017/04/headless-chrome
                // https://bitbucket.org/chromiumembedded/cef/issues/2349/headless-mode-without-x
        
        const char* arg_soup[] = { "-disable-surfaces",
        "-enable-gpu", 
        //"-disable-gpu-compositing",
        "-enable-begin-frame-scheduling",
        "-disable-software-rasterizer"};        

        for (const char* a : arg_soup){
            command_line->AppendSwitch(a);
        }

        args_loaded = true;
    }
private:
    IMPLEMENT_REFCOUNTING(AppWrapper);
};


class RenderHandler :
    public CefRenderHandler
{
public:
    unsigned char* texture_data;

    RenderHandler(int w, int h) :
        width(w),
        height(h)
    {
        // https://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#OnPaint(CefRefPtr%3CCefBrowser%3E,PaintElementType,constRectList&,constvoid*,int,int)
        // |buffer| will be |width|*|height|*4 bytes in size and represents a BGRA image with an upper-left origin
        texture_data = (unsigned char *) malloc(w * h * 4);
    }

    ~RenderHandler()
    {
        texture = nullptr;
        renderer = nullptr;
    }

    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
    {
        //std::cout << "void RenderHandler::GetViewRect\n";


        rect = CefRect(0, 0, width, height);
    }

    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int w, int h)
    {
        //std::cout << "void RenderHandler::OnPaint\n";
        int texture_pitch = 0;
        // https://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#OnPaint(CefRefPtr%3CCefBrowser%3E,PaintElementType,constRectList&,constvoid*,int,int)
        // |buffer| will be |width|*|height|*4 bytes in size and represents a BGRA image with an upper-left origin

        // TODO: Optimize this operation
        for (int i = 0; i < w * h * 4; i++) {
            if (i % 4 == 0) {
                // Blue to red
                texture_data[i + 2] = ((char*)buffer)[i];
            }
            else if (i % 4 == 2) {
                // Red to blue
                texture_data[i - 2] = ((char*)buffer)[i];
            }
            else {
                texture_data[i] = ((char*)buffer)[i];
            }
        }

        //memcpy(texture_data, buffer, w * h * 4);
    }

    void resize(int w, int h)
    {
        texture = nullptr;
        width = w;
        height = h;
        free(texture_data);
        texture_data = (unsigned char*)malloc(w * h * 4);
    }

    void render()
    {
    }

private:
    int width;
    int height;
    void* renderer = nullptr;
    void* texture = nullptr;

    IMPLEMENT_REFCOUNTING(RenderHandler);
};

class LoadHandler :
    public CefLoadHandler {
public:
    bool success = false;
        LoadHandler()
        {
        }

        ~LoadHandler()
        {
        }

        void OnLoadingStateChange(CefRefPtr< CefBrowser > browser, bool isLoading, bool canGoBack, bool canGoForward)
        {
            //std::cout << "void LoadHandler::OnLoadingStateChange\n";
            if (!isLoading) {
                success = true;
            }
        }

private:

    IMPLEMENT_REFCOUNTING(LoadHandler);
};

// for manual render handler
class BrowserClient :
    public CefClient,
    public CefLifeSpanHandler,
    public CefLoadHandler
{
public:
    BrowserClient(CefRefPtr<CefRenderHandler> ptr) :
        rend_handler(ptr)
    {
    }

    BrowserClient(CefRefPtr<CefRenderHandler> rendPtr, CefRefPtr<CefLoadHandler> loadPtr, CefRefPtr<CefAudioHandler> audioPtr) :
        rend_handler(rendPtr),
        load_handler(loadPtr),
        audio_handler(audioPtr)
    {
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler()
    {
        //std::cout << "void BrowserClient::GetLifeSpanHandler\n";
        return this;
    }

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler()
    {
        //std::cout << "void BrowserClient::GetLoadHandler\n";
        return load_handler;
    }

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
    {
        //std::cout << "void BrowserClient::GetRenderHandler\n";
        return rend_handler;
    }

    virtual CefRefPtr<CefAudioHandler> GetAudioHandler() {
        return audio_handler;
    }
    // CefLifeSpanHandler methods.
    void OnAfterCreated(CefRefPtr<CefBrowser> browser)
    {
        // Must be executed on the UI thread.
        CEF_REQUIRE_UI_THREAD();

        browser_id = browser->GetIdentifier();
    }

    bool DoClose(CefRefPtr<CefBrowser> browser)
    {
        // Must be executed on the UI thread.
        CEF_REQUIRE_UI_THREAD();

        // Closing the main window requires special handling. See the DoClose()
        // documentation in the CEF header for a detailed description of this
        // process.
        if (browser->GetIdentifier() == browser_id)
        {
            // Set a flag to indicate that the window close should be allowed.
            closing = true;
        }

        // Allow the close. For windowed browsers this will result in the OS close
        // event being sent.
        return false;
    }

    void OnBeforeClose(CefRefPtr<CefBrowser> browser)
    {
    }

    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
    {
        //std::cout << "OnLoadEnd(" << httpStatusCode << ")" << std::endl;
        //std::cout << "   loaded set to true" << std::endl;
        loaded = true;
    }

    bool OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& failedUrl, CefString& errorText)
    {
        //std::cout << "OnLoadError()" << std::endl;
        loaded = true;
    }

    void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
    {
        //std::cout << "OnLoadingStateChange()" << std::endl;
    }

    void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
    {
        //std::cout << "OnLoadStart()" << std::endl;
    }

    bool closeAllowed() const
    {
        return closing;
    }

    bool isLoaded() const
    {
        return loaded;
    }

private:
    int browser_id;
    bool closing = false;
    bool loaded = false;
    CefRefPtr<CefRenderHandler> rend_handler;
    CefRefPtr<CefLoadHandler> load_handler;
    CefRefPtr<CefAudioHandler> audio_handler;
    IMPLEMENT_REFCOUNTING(BrowserClient);
};

/*
CefBrowserHost::MouseButtonType translateMouseButton(SDL_MouseButtonEvent const &e)
{
    CefBrowserHost::MouseButtonType result;
    switch (e.button)
    {
    case SDL_BUTTON_LEFT:
    case SDL_BUTTON_X1:
        result = MBT_LEFT;
        break;

    case SDL_BUTTON_MIDDLE:
        result = MBT_MIDDLE;
        break;

    case SDL_BUTTON_RIGHT:
    case SDL_BUTTON_X2:
        result = MBT_RIGHT;
        break;
    }
    return result;
}*/
/*
int cef_handler_setup()
{
    CefMainArgs args;
    CefSettings settings;

    int exec_result = CefExecuteProcess(args, nullptr, nullptr);
    // checkout CefApp, derive it and set it as second parameter, for more control on
    // command args and resources.
    if (exec_result >= 0) // child proccess has endend, so exit.
    {
        return exec_result;
    }
    else if (exec_result == -1)
    {
        // we are here in the father proccess.
    }


    // CefString(&settings.resources_dir_path).FromASCII("");
    
    std::ostringstream ss;
    // ss << SDL_GetBasePath() << "locales/";
    CefString(&settings.locales_dir_path) = ss.str();
    

    CefString(&settings.locales_dir_path).FromASCII("");
    //CefString(&settings.resources_dir_path) = SDL_GetBasePath();

    // checkout detailed settings options http://magpcss.org/ceforum/apidocs/projects/%28default%29/_cef_settings_t.html
    // nearly all the settings can be set via args too.
    // settings.multi_threaded_message_loop = true; // not supported, except windows
    // CefString(&settings.browser_subprocess_path).FromASCII("sub_proccess path, by default uses and starts this executeable as child");
    // CefString(&settings.cache_path).FromASCII("");
    // CefString(&settings.log_file).FromASCII("");
    // settings.log_severity = LOGSEVERITY_DEFAULT;

    bool init_result = CefInitialize(args, settings, nullptr, nullptr);
    
    // CefInitialize creates a sub-proccess and executes the same executeable, as calling CefInitialize, if not set different in settings.browser_subprocess_path
    // if you create an extra program just for the childproccess you only have to call CefExecuteProcess(...) in it.
    
    if (!init_result)
    {
        // handle error
        return -1;
    }

    //Initialize SDL
    /*if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    int width = 800;
    int height = 600;

    //auto window = SDL_CreateWindow("Render CEF with SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE);
    CefRefPtr<RenderHandler> renderHandler = new RenderHandler(width, height);

    // create browser-window
    CefRefPtr<CefBrowser> browser;
    CefRefPtr<BrowserClient> browserClient;

    {
        CefWindowInfo window_info;
        CefBrowserSettings browserSettings;

        // browserSettings.windowless_frame_rate = 60; // 30 is default

        window_info.SetAsWindowless(NULL); // false means no transparency (site background colour)

        browserClient = new BrowserClient(renderHandler);

        browser = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), "http://www.google.com", browserSettings, nullptr, nullptr);

        // inject user-input by calling - non-trivial for non-windows - checkout the cefclient source and the platform specific cpp, like cefclient_osr_widget_gtk.cpp for linux
        // browser->GetHost()->SendKeyEvent(...);
        // browser->GetHost()->SendMouseMoveEvent(...);
        // browser->GetHost()->SendMouseClickEvent(...);
        // browser->GetHost()->SendMouseWheelEvent(...);
    }

    bool shutdown = false;
    bool js_executed = false;
    int null_event = 0;
    while (!browserClient->closeAllowed())
    {
        // send events to browser        
        while (!shutdown && null_event != 0)
        {
            switch (null_event)
            {
            case 0: { //SDL_QUIT
                shutdown = true;
                browser->GetHost()->CloseBrowser(false);
                break; }
            case 1: {//SDL_KEYDOWN:
                CefKeyEvent event;
                //event.modifiers = getKeyboardModifiers(e.key.keysym.mod);
                //event.windows_key_code = getWindowsKeyCode(e.key.keysym);

                event.type = KEYEVENT_RAWKEYDOWN;
                browser->GetHost()->SendKeyEvent(event);

                event.type = KEYEVENT_CHAR;
                browser->GetHost()->SendKeyEvent(event);
                break; }
            case 2: {//SDL_KEYUP:
                CefKeyEvent event;
                //event.modifiers = getKeyboardModifiers(e.key.keysym.mod);
                //event.windows_key_code = getWindowsKeyCode(e.key.keysym);

                event.type = KEYEVENT_KEYUP;

                browser->GetHost()->SendKeyEvent(event);
                break; }
            case 3: {//SDL_WINDOWEVENT:
                int null_window_event = 0;
                switch (null_window_event) {
                case 0://SDL_WINDOWEVENT_SIZE_CHANGED:
                    //renderHandler->resize(e.window.data1, e.window.data2);
                    browser->GetHost()->WasResized();
                    break;
                case 1://SDL_WINDOWEVENT_FOCUS_GAINED:
                    browser->GetHost()->SetFocus(true);
                    break;
                case 2://SDL_WINDOWEVENT_FOCUS_LOST:
                    browser->GetHost()->SetFocus(false);
                    break;
                case 3://SDL_WINDOWEVENT_HIDDEN:
                case 4://SDL_WINDOWEVENT_MINIMIZED:
                    // Deprecated?
                    //browser->GetHost()->SetWindowVisibility(false);

                    browser->GetHost()->WasHidden(true);
                    break;
                case 5://SDL_WINDOWEVENT_SHOWN:
                case 6://SDL_WINDOWEVENT_RESTORED:
                    //browser->GetHost()->SetWindowVisibility(true);
                    browser->GetHost()->WasHidden(false);
                    break;
                case 7://SDL_WINDOWEVENT_CLOSE:
                    //e.type = SDL_QUIT;
                    //SDL_PushEvent(&e);
                    break;
                }
                break; }
            case 4: {//SDL_MOUSEMOTION:
                CefMouseEvent event;
                //event.x = e.motion.x;
                //event.y = e.motion.y;

                browser->GetHost()->SendMouseMoveEvent(event, false);
                break; }
            case 5: {//SDL_MOUSEBUTTONUP:
                CefMouseEvent event;
                //event.x = e.button.x;
                //event.y = e.button.y;

               // browser->GetHost()->SendMouseClickEvent(event, translateMouseButton(e.button), true, 1);
                break; }
            case 6: {//SDL_MOUSEBUTTONDOWN:
                CefMouseEvent event;
                //event.x = e.button.x;
                //event.y = e.button.y;

                //browser->GetHost()->SendMouseClickEvent(event, translateMouseButton(e.button), false, 1);
                break; }
            case 7: {//SDL_MOUSEWHEEL:
                //int delta_x = e.wheel.x;
                //int delta_y = e.wheel.y;

                /*if (SDL_MOUSEWHEEL_FLIPPED == e.wheel.direction)
                {
                    delta_y *= -1;
                }
                else
                {
                    delta_x *= -1;
                }
                int delta_x = 0;
                int delta_y = 0;
                CefMouseEvent event;
                browser->GetHost()->SendMouseWheelEvent(event, delta_x, delta_y);
                break; }
            }
        }

        if (!js_executed && browserClient->isLoaded())
        {
            js_executed = true;

            CefRefPtr<CefFrame> frame = browser->GetMainFrame();
            frame->ExecuteJavaScript("alert('ExecuteJavaScript works!');", frame->GetURL(), 0);
        }

        // let browser process events
        CefDoMessageLoopWork();

        // render
        //SDL_RenderClear(renderer);

        renderHandler->render();

        // Update screen
        //SDL_RenderPresent(renderer);
    }

    browser = nullptr;
    browserClient = nullptr;
    renderHandler = nullptr;

    CefShutdown();

    //SDL_DestroyRenderer(renderer);


    //SDL_DestroyWindow(window);
    //SDL_Quit();

    return 0;
}
*/ 

#endif#pragma_once