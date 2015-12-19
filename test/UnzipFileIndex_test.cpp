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

