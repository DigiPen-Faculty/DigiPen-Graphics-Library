# DigiPen Graphics Library

This is a simple library to handle input, graphics, and the application window. The interface is designed for use in programs written in C.

A zip file with the current version of the library files can be found on the [Releases](https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/releases) page. 

The repository is structured as follows:
- The header, `.DLL`, and `.lib` files can be found in the [DGL folder](./DGL/). The current released version of this folder is on the [Releases](https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/releases) page. 
- The [DGL Template Project](./DGL_Template_Project/) is set up to access the files in the DGL folder and can be used as an example of Visual Studio project settings. There is also a documentation page on [creating new Visual Studio projects](https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/wiki/Visual-Studio-Projects).
- The [DigiPen Graphics Library folder](./DigiPen_Graphics_Library/) contains the source code for the DGL. 
- `DGL.sln` in the root folder is a solution which contains both the template and the DGL projects.
- `CMakeLists.txt` in the root folder builds the DGL, the template, and the test program on Windows, Linux, and macOS.

## Building with CMake (Windows, Linux, and macOS)

You need:
- The [Vulkan SDK](https://vulkan.lunarg.com), with the `VULKAN_SDK` environment variable set. It provides the Vulkan loader, the DXC shader compiler, and on macOS, MoltenVK (which runs Vulkan on top of Metal). On macOS, choose the system global installation so programs can find MoltenVK.
- [vcpkg](https://vcpkg.io), with the `VCPKG_ROOT` environment variable set. It downloads GLFW, stb, and the Vulkan Memory Allocator.
- On Linux, the packages GLFW needs to build, for example on Ubuntu: `sudo apt install pkg-config libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev libwayland-dev libxkbcommon-dev wayland-protocols`.

Then, from the root folder:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

The library, the DXC library, the programs, and their `shaders` folders are put in `build/bin` (`build/bin/Debug` with Visual Studio). Run the programs from that folder, since the default shaders are loaded from `shaders/` relative to the working folder. `DGL_Test --selftest` runs the automatic checks.

Platform differences:
- The keyboard and mouse checks in the self-test only run on Windows.
- `createConsole` has no effect outside Windows. Output goes to the terminal the program was started from.
- macOS and Wayland (Linux) don't support window icons, and Wayland doesn't let programs set the window position. These calls are ignored there.
- `DGL_System_GetNativeWindowHandle()` returns NULL outside Windows.

Documentation can be found on the [wiki](https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/wiki) or in the [docs folder](./docs/).

Feel free to create issues for bug reports or feature requests, or to submit pull requests, either for the code or for the documentation.
