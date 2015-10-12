/* 
 * File:   UnzipFileIndex_test.cpp
 * Author: alex
 *
 * Created on October 12, 2015, 5:58 AM
 */

#include <iostream>
#include <cassert>

#include "staticlib/unzip/UnzipFileIndex.hpp"

namespace uz = staticlib::unzip;

void test_entries() {
    uz::UnzipFileIndex idx{"../test/bundle.zip"};
    auto desc_aaa = idx.find_zip_entry("bundle/aaa.txt");
    assert(144 == desc_aaa.offset);
    assert(4 == desc_aaa.comp_length);
    assert(4 == desc_aaa.uncomp_length);
    assert(0 == desc_aaa.comp_method);
    auto desc_bbbb = idx.find_zip_entry("bundle/bbbb.txt");
    (void) desc_bbbb;
    assert(65 == desc_bbbb.offset);
    assert(6 == desc_bbbb.comp_length);
    assert(9 == desc_bbbb.uncomp_length);
    assert(8 == desc_bbbb.comp_method);
    auto desc_fail = idx.find_zip_entry("bundle/fail.txt");
    (void) desc_fail;
    assert(-1 == desc_fail.offset);
    assert(-1 == desc_fail.comp_length);
    assert(-1 == desc_fail.uncomp_length);
    assert(0 == desc_fail.comp_method);    
}

int main() {
    test_entries();

    return 0;
}

