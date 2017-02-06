/*
 * Copyright 2015, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   operations_test.cpp
 * Author: alex
 *
 * Created on October 12, 2015, 10:01 AM
 */


#include "staticlib/unzip.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <array>

#include "staticlib/config/assert.hpp"

#include "staticlib/io.hpp"

namespace uz = staticlib::unzip;
namespace io = staticlib::io;

void test_read_inflate() {
    uz::UnzipFileIndex idx{"../test/data/bundle.zip"};
    std::ostringstream out{};
    io::streambuf_sink sink{out.rdbuf()};
    auto ptr = uz::open_zip_entry(idx, "bundle/bbbb.txt");
    io::streambuf_source src{ptr.get()};
    std::array<char, 4096> buf{{}};
    io::copy_all(src, sink, buf);
    // std::cout << "[" << out.str() << "]" << std::endl;
    slassert("bbbbbbbb\n" == out.str());
}

void test_read_store() {
    uz::UnzipFileIndex idx{"../test/data/bundle.zip"};
    std::ostringstream out{};
    io::streambuf_sink sink{out.rdbuf()};
    auto ptr = uz::open_zip_entry(idx, "bundle/aaa.txt");
    io::streambuf_source src{ptr.get()};
    std::array<char, 4096> buf{{}};
    io::copy_all(src, sink, buf);
    // std::cout << "[" << out.str() << "]" << std::endl;
    slassert("aaa\n" == out.str());
}

void test_read_manual() {
    uz::UnzipFileIndex idx{"../test/data/test.zip"};
    slassert(2 == idx.get_entries().size());
    slassert("foo.txt" == idx.get_entries()[0]);
    slassert("bar/baz.txt" == idx.get_entries()[1]);
    std::ostringstream out{};
    io::streambuf_sink sink{out.rdbuf()};
    auto ptr = uz::open_zip_entry(idx, "bar/baz.txt");
    io::streambuf_source src{ptr.get()};
    std::array<char, 4096> buf{{}};
    io::copy_all(src, sink, buf);
//    std::cout << "[" << out.str() << "]" << std::endl;
    slassert("bye" == out.str());
}

int main() {
    try {
        test_read_inflate();
        test_read_store();
        test_read_manual();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
