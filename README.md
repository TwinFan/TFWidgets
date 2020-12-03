# TFWidgets
Simple widgets for modern XP11 windows, i.e. without using XP's old-style widgets.

## Note: Outdated, archived!

For new projects consider using imgui. A sample implementation is available as [sparker256/imgui4xp](https://github.com/sparker256/imgui4xp).

## TFW files

You only need to include 4 files into your own projects:
- `TFW.cpp`
- `TFW.h`
- `TFWGraphcs.cpp`
- `TFWGraphcs.h`

All definitions are encapsulated in the namespace `TFW`.

All files include lots of comments in Doxygen style as documentation.
The generated Doxygen documentation is available in `Documentation/html/index.html`.

## Example Plugin Implementation

The `Example` folder includes an example X-Plane plugin
(based on the [Hello World plugin](https://developer.x-plane.com/code-sample/hello-world-sdk-3/)),
which demonstrates how to use TFWidgets.

The plugin just opens a windows with lots of example widgets. It doesn't look nice any
longer...but it has lots of widgets ;-)

## Building the Example

### Docker

A docker environment to build all 3 platforms is included.

- Install [Docker Desktop](https://www.docker.com/products/docker-desktop)
- In a terminal window, change directory to `TFWidgets/Example/docker`
- Type `make`

The docker environment will be downloaded and installed, which takes several
minutes, but is needed only once. Thereafter the actual build takes place.

Resulting binaries are in `TFWidgets/Example/build-[lin/mac/win]`

### IDE

- Mac: Open the XCode project `TFWidgets.xcodeproj`. There might be paths you way want to check...especially the path the resulting plugin is copied to after build (Targets > Build Phases > Copy Files).
- Windows: Open the Visual Studio solution `TFWidgets.sln`. Also here you might need to check directories. There is a copy command to my X-Plane installation in the _Post Build Event_.
