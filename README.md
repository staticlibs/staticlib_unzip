Staticlibs unZIP library
========================

This project is a part of [Staticlibs](http://staticlibs.net/).

This project allows to open "input streams" to entries inside the ZIP file.

Link to the [API documentation](http://staticlibs.net/staticlib_unzip/docs/html/namespacestaticlib_1_1unzip.html).

How to build
------------

[CMake](http://cmake.org/) is required for building.

[pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/) utility is used for dependency management.
For Windows users ready-to-use binary version of `pkg-config` can be obtained from [tools_windows_pkgconfig](https://github.com/staticlibs/tools_windows_pkgconfig) repository.
See [PkgConfig](https://github.com/staticlibs/wiki/wiki/PkgConfig) for Staticlibs-specific details about `pkg-config` usage.

This project depends on a [zlib](https://github.com/madler/zlib) and on a number of Staticlib libraries.

See [StaticlibsDependencies](https://github.com/staticlibs/wiki/wiki/StaticlibsDependencies) for more 
details about dependency management with Staticlibs.

To build this project manually:

 * checkout all the dependent projects
 * configure these projects using the same output directory:

Run:

    mkdir build
    cd build
    cmake .. -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=<my_lib_dir>

 * build all the dependent projects
 * configure this projects using the same output directory and build it:

To build the library on Windows using Visual Studio 2013 Express run the following commands using
Visual Studio development command prompt 
(`C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\Shortcuts\VS2013 x86 Native Tools Command Prompt`):

    git clone https://github.com/staticlibs/staticlib_unzip.git
    cd staticlib_unzip
    mkdir build
    cd build
    cmake .. -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=<my_lib_dir>
    msbuild staticlib_unzip.sln

To build on other platforms using GCC or Clang with GNU Make:

    cmake .. -DCMAKE_CXX_FLAGS="--std=c++11"
    make

See [StaticlibsToolchains](https://github.com/staticlibs/wiki/wiki/StaticlibsToolchains) for 
more information about the toolchain setup and cross-compilation.

License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

Changelog
---------

**2015-12-03**

 * version 1.0.1
 * deplibs cache support
 * headers cleanup

**2015-11-01**

 * version 1.0.0 - initial public version

