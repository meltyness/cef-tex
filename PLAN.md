

- [] Fix build system
	- [] User tasks
		- [] Fetch cef-tex from Git
		- [] Follow simple instructions to kick-off build system
			- [] I.E. CMake, Jenkins, Python script
			- [] CMake is necessary for cef-project, pulls cef binary dist 

	- [] Basic tasks
		- [] Fetch CEF from Git
			- [] git clone https://bitbucket.org/chromiumembedded/cef-project.git
			- [] verify success
			- [] cmake .
			- [] verify success
			- [] Use MSBuild to build ALL_BUILD.vcxproj
				- [] preferably with -maxCpuCount:16
				- [] ... must have -p:Configuration=Release
		- [] Build for specific platform
		- [] Fetch Godot from Git
			- [] definitely in scope
			- [] https://github.com/godotengine/godot-cpp
			- [] Grab the 3.x branch, for now.
			- [] it needs to be built too which is hard and depends on Scons, ouch
		- [] Library directories for CEF, and Godot-cpp into cef-tex build
		- [] Include directories for CEF, and Godot-cpp into cef-tex build
		- [] Build cef-tex for specific platform
			- [] Cef-tex also somehow needs to correctly inherit cefclient binary location
			- [] Also what in the fuck is the fucking search-path for the fucking dlls fucking
			     cefclient fucking needs when it's being fucking loaded by fucking ceftex. fuck!
			- [] So
				User calls Godot -> CEFTex.dll -> cefclient.exe
				They pretty much all need to be in the same directory, for simplicity.
				Build 'em, and mash 'em together. It's fine, it's part of the build.
		    - [] Whatever, just fucking dump it in there, holy shit.
		- [] Configure gdns/gdnlib for the DLL

