/* 
 * File:   operations_test.cpp
 * Author: alex
 *
 * Created on October 12, 2015, 10:01 AM
 */

#include <iostream>
#include <string>
#include <sstream>
#include <array>
#include <cassert>

#include "staticlib/unzip.hpp"
#include "staticlib/io.hpp"

namespace uz = staticlib::unzip;
namespace io = staticlib::io;

void test_read_inflate() {
    uz::UnzipFileIndex idx{"../test/bundle.zip"};
    std::ostringstream out{};
    io::streambuf_sink sink{out.rdbuf()};
    auto ptr = uz::open_zip_entry(idx, "bundle/bbbb.txt");
    io::streambuf_source src{ptr.get()};
    std::array<char, 8192> buf{{}};
    io::copy_all(src, sink, buf.data(), buf.size());
    // std::cout << "[" << out.str() << "]" << std::endl;
    assert("bbbbbbbb\n" == out.str());
}

void test_read_store() {
    uz::UnzipFileIndex idx{"../test/bundle.zip"};
    std::ostringstream out{};
    io::streambuf_sink sink{out.rdbuf()};
    auto ptr = uz::open_zip_entry(idx, "bundle/aaa.txt");
    io::streambuf_source src{ptr.get()};
    std::array<char, 8192> buf{{}};
    io::copy_all(src, sink, buf.data(), buf.size());
    // std::cout << "[" << out.str() << "]" << std::endl;
    assert("aaa\n" == out.str());
}


int main() {
    test_read_inflate();
    test_read_store();

    return 0;
}

