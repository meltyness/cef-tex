... this is really ugly.

# cef-tex introduction
Cef-tex is a C++-based proof-of-concept grade module for Godot game engine that 
facilitates instantiating web browser processes, receive audio from them, 
and renders it to a texture for usage in any Godot-based applications. 

Cef-tex originated from a backhanded challenge someone issued regarding a previous 
implementation that used PythonCEF, and without significant modification to the
underlying, abandoned project, wouldn't be able to grab the audio stream. After
numerous attempts to make PythonCEF's build system operable it was resolved to 
simply use CEF directly in this way. Things were much simpler this way, and if
audio from the browser isn't needed, I recommend doing it this way.

Here's an annoying illustration captured in Godot: https://www.youtube.com/watch?v=-Pw1xs2w3gc

# dependencies
It uses the following 
- Chromium Embedded Framework (https://bitbucket.org/chromiumembedded/cef/src/master/)
- Bastian Olij's Godot OpenXR Plugin (https://github.com/GodotVR/godot_openxr)

You'll likely need to grab: `src/CEFTex/x64/Release/libcef.dll`
in order for this to work. Check the CEF repo listed above to figure that one out.

# how it works
The basic architecture is that the C++ code provides a way to operate a `cef_sandbox`
in the background, and client code extracts audio/video updates (by implementing CefApp, 
CefAudioHandler, CefRenderHandler, etc...) and sends some input to make the browser
interactable.

`sdl-stripped-cpp-example.txt` illustrates bootstrapping

# code status
This was built with Godot 3.4.3.

This is a limited part of what's really needed, I haven't included the full scene demonstrated
above, but here is a basic usage scenario: 

![image](https://user-images.githubusercontent.com/86126050/234715788-e95f2532-191d-446e-a0f3-f12bef367d48.png)

- A *control* contains a CefTexInputWrapper (and script) 
- The CefTexInputWrapper contains CefTex, a *TextureRect* and a `gdns` which references the `gdnlib` which in turn
     references the compiled C++ module
- There's also a *Control* containing the back button, and a signal connection to the InputWrapper
- Finally the Node, AudioManager, contains a script which receives an unseen AudioStreamPlayer32, and every tick
     checks CefTex library for available sample buffer, and pushes it into the scene.

Keyboard / text input is complicated by OS integration, the project doesn't support this
but if you want to use HTML5 as your toolchain for your menu and maybe signal back to
Godot encoding commands into the URLs traversed, this would be a good tool to do that with.

# building
Some pre-built binaries included, and others are not. The VC++ project should illustrate the requirements,
and Godot has docs on how to use C++. They won't work because of the following TODO in `Implementation.cpp`

`// TODO: Figure out how to fucking mke this a relative path`

It should be compatible with Linux, but you're on your own with building, and 
reconfiguring the references to the link libraries.

It has been a full year since I made any effort on this project, and I remember 
little about what needed to be done, but a few things spring to mind:
- When compiling the library there was a particular need to use static/dynamic single/multi-threaded library compilation, 
  - Those are in the VC solution files somewhere, it's XML so human readable, but not a choice build-system.
- When delivering the CEF library and the DLLs for the texture to Godot, those options are a part of `main.tscn`

# changing
Primary implementation logic unqiue to this project lies in the following source files:
`src/cef-tex/Implementation.cpp`
`src/cef-tex/cef_handler.cpp`
`cef_generator.gd`

# licensing
The relevant sections are released under MIT license, and subject to restrictions by licenses of some included components that may or may not be redistributed.

Any sections not subject to the MIT license are subject to their respective licenses.

```
// Copyright (c) 2008-2020 Marshall A. Greenblatt. Portions Copyright (c)
// 2006-2009 Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the name Chromium Embedded
// Framework nor the names of its contributors may be used to endorse
// or promote products derived from this software without specific prior
// written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
