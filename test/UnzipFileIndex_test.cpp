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
 * File:   UnzipFileIndex_test.cpp
 * Author: alex
 *
 * Created on October 12, 2015, 5:58 AM
 */

#include "staticlib/unzip/UnzipFileIndex.hpp"

#include <iostream>

#include "staticlib/config/assert.hpp"


namespace uz = staticlib::unzip;

void test_entries() {
    uz::UnzipFileIndex idx{"../test/bundle.zip"};
    auto desc_aaa = idx.find_zip_entry("bundle/aaa.txt");
    slassert(144 == desc_aaa.offset);
    slassert(4 == desc_aaa.comp_length);
    slassert(4 == desc_aaa.uncomp_length);
    slassert(0 == desc_aaa.comp_method);
    auto desc_bbbb = idx.find_zip_entry("bundle/bbbb.txt");
    slassert(65 == desc_bbbb.offset);
    slassert(6 == desc_bbbb.comp_length);
    slassert(9 == desc_bbbb.uncomp_length);
    slassert(8 == desc_bbbb.comp_method);
    auto desc_fail = idx.find_zip_entry("bundle/fail.txt");
    slassert(-1 == desc_fail.offset);
    slassert(-1 == desc_fail.comp_length);
    slassert(-1 == desc_fail.uncomp_length);
    slassert(0 == desc_fail.comp_method);    
}

int main() {
    try {
        test_entries();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
