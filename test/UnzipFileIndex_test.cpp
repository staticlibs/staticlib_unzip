/* 
 * File:   UnzipFileIndex_test.cpp
 * Author: alex
 *
 * Created on October 12, 2015, 5:58 AM
 */

#include <iostream>

#include "staticlib/unzip/UnzipFileIndex.hpp"

namespace uz = staticlib::unzip;

int main() {
    uz::UnzipFileIndex idx{"../test/bundle.zip"};
    auto desc_aaa = idx.find_zip_entry("bundle/aaa.txt");
    std::cout << desc_aaa.first << " " << desc_aaa.second << std::endl;
    auto desc_bbbb = idx.find_zip_entry("bundle/bbbb.txt");
    std::cout << desc_bbbb.first << " " << desc_bbbb.second << std::endl;
    auto desc_fail = idx.find_zip_entry("bundle/fail.txt");
    std::cout << desc_fail.first << " " << desc_fail.second << std::endl;

    return 0;
}

