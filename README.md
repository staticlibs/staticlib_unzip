Staticlibs unZIP library
========================

[![travis](https://travis-ci.org/staticlibs/staticlib_unzip.svg?branch=master)](https://travis-ci.org/staticlibs/staticlib_unzip)
[![appveyor](https://ci.appveyor.com/api/projects/status/github/staticlibs/staticlib_unzip?svg=true)](https://ci.appveyor.com/project/staticlibs/staticlib-unzip)

This project is a part of [Staticlibs](http://staticlibs.net/).

This project allows to open "input streams" to entries inside the ZIP file. [staticlib_compress](https://github.com/staticlibs/staticlib_compress) library can be used to write ZIP files.

Link to the [API documentation](http://staticlibs.net/staticlib_unzip/docs/html/namespacestaticlib_1_1unzip.html).

How to build
------------

[CMake](http://cmake.org/) is required for building.

[pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/) utility is used for dependency management.
For Windows users ready-to-use binary version of `pkg-config` can be obtained from [tools_windows_pkgconfig](https://github.com/staticlibs/tools_windows_pkgconfig) repository.
See [StaticlibsPkgConfig](https://github.com/staticlibs/wiki/wiki/StaticlibsPkgConfig) for Staticlibs-specific details about `pkg-config` usage.

To build the library on Windows using Visual Studio 2013 Express run the following commands using
Visual Studio development command prompt 
(`C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\Shortcuts\VS2013 x86 Native Tools Command Prompt`):

    git clone --recursive https://github.com/staticlibs/external_zlib.git
    git clone https://github.com/staticlibs/staticlib_config.git
    git clone https://github.com/staticlibs/staticlib_io.git
    git clone https://github.com/staticlibs/staticlib_endian.git
    git clone https://github.com/staticlibs/staticlib_pimpl.git
    git clone https://github.com/staticlibs/staticlib_utils.git
    git clone https://github.com/staticlibs/staticlib_compress.git
    git clone https://github.com/staticlibs/staticlib_unzip.git
    cd staticlib_unzip
    mkdir build
    cd build
    cmake .. -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=<my_lib_dir>
    msbuild staticlib_unzip.sln

To build on other platforms using GCC or Clang with GNU Make:

    cmake .. -DCMAKE_CXX_FLAGS="--std=c++11"
    make

Cloning of [external_zlib](https://github.com/staticlibs/external_jansson.git) is not required on Linux - 
system Zlib library will be used instead.

See [StaticlibsToolchains](https://github.com/staticlibs/wiki/wiki/StaticlibsToolchains) for 
more information about the toolchain setup and cross-compilation.

License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

Changelog
---------

**2017-06-12**

 * version 1.0.7
 * use `istream` source

**2017-12-24**

 * version 1.0.6
 * use `inflate_source` from `staticlib_compress`
 * vs2017 support

**2017-05-19**

 * version 1.0.5
 * deps update

**2016-06-29**

 * version 1.0.5
 * ICU API removed

**2016-01-22**

 * version 1.0.4
 * minor CMake changes

**2015-12-19**

 * version 1.0.3
 * fix `inflate` return check

**2015-12-04**

 * version 1.0.2
 * optional ICU support

**2015-12-03**

 * version 1.0.1
 * deplibs cache support
 * headers cleanup

**2015-11-01**

 * version 1.0.0 - initial public version

