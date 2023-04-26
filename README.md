# cef-tex
Cef-tex is a C++-based proof-of-concept grade module for Godot game engine that 
facilitates instantiating web browser processes, receive audio from them, 
and renders it to a texture for usage in any Godot-based applications. 

It uses the following 
- Chromium Embedded Framework (https://bitbucket.org/chromiumembedded/cef/src/master/)
- Bastian Olij's Godot OpenXR Plugin (https://github.com/GodotVR/godot_openxr)

In trying to get the codebase merged, it occurs that you'll likely need to grab:
`src/CEFTex/x64/Release/libcef.dll`

in order for this to work. Check the CEF repo listed above to figure that one out.

Here's an annoying illustration captured in Godot: https://www.youtube.com/watch?v=-Pw1xs2w3gc

There are some improvements pushed here, the controller code transitioned to a 
properly-determined state machine instead of some hacky stuff slapped together
to focus on other components, and instead of the back button being the Godot-bot,
instead it's a back symbol.

It originated from a backhanded challenge someone issued regarding a previous 
implementation that used PythonCEF, and without significant modification to the
underlying, abandoned project, wouldn't be able to grab the audio stream. After
numerous attempts to make PythonCEF's build system operable it was resolved to 
simply use CEF directly in this way. Things were much simpler this way, and if
audio from the browser isn't needed, I recommend doing it this way.

The basic architecture is that the C++ code provides a way to operate a `cef_sandbox`
in the background, and client code extracts audio/video updates (by implementing CefApp, 
CefAudioHandler, CefRenderHandler, etc...) and sends some input to make the browser
interactable.

It has been a full year since I made any effort on this project, and I remember 
little about what needed to be done, but a few things spring to mind:
- When compiling the library there was a particular need to use static/dynamic single/multi library compilation, those are in the VC solution files somewhere.
- When delivering the CEF library and the DLLs for the texture to Godot, those options are a part of `main.tscn`

Primary implementation logic unqiue to this project lies in the following source files:
`src/cef-tex/Implementation.cpp`
`src/cef-tex/cef_handler.cpp`
`cef_generator.gd`

This is released under MIT license, and subject to restrictions by licenses of some included components that may or may not be redistributed.
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
