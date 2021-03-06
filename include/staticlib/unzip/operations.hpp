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
 * File:   operations.hpp
 * Author: alex
 *
 * Created on October 8, 2015, 7:12 PM
 */

#ifndef STATICLIB_UNZIP_OPERATIONS_HPP
#define STATICLIB_UNZIP_OPERATIONS_HPP

#include <memory>
#include <istream>
#include <string>

#include "staticlib/unzip/file_index.hpp"

namespace staticlib {
namespace unzip {

/**
 * Opens "input stream" to the specified ZIP entry in the ZIP file corresponding to
 * the specified index
 * 
 * @param idx ZIP file index
 * @param entry_name ZIP entry name
 * @return unique pointer to the unbuffered streambuf
 */
std::unique_ptr<std::istream> open_zip_entry(const file_index& idx, const std::string& entry_name);

} // namespace
}

#endif /* STATICLIB_UNZIP_OPERATIONS_HPP */

