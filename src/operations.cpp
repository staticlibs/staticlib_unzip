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
 * File:   operations.cpp
 * Author: alex
 * 
 * Created on October 11, 2015, 10:21 PM
 */

#include "staticlib/unzip/operations.hpp"

#include <cstring>
#include <algorithm>
#include <array>
#include <ios>
#include <string>
#include <memory>

// http://stackoverflow.com/a/1904659/314015
#define NOMINMAX

#include "staticlib/config.hpp"
#include "staticlib/endian.hpp"
#include "staticlib/io.hpp"
#include "staticlib/compress.hpp"
#include "staticlib/utils.hpp"
#include "staticlib/tinydir.hpp"

#include "staticlib/unzip/unzip_exception.hpp"


namespace staticlib {
namespace unzip {

namespace { // anonymous

const uint32_t zip_cd_start_signature = 0x04034b50;

class unzip_entry_source {
    using tinydir_file_ref_type = sl::io::reference_source<sl::tinydir::file_source>;
    using inflater_type = sl::compress::inflate_source<tinydir_file_ref_type>;

    std::string zip_file_path;
    std::string zip_entry_name;
    file_entry entry; 
    sl::tinydir::file_source fd;
    std::unique_ptr<inflater_type> inflater;

    size_t avail_out;

public:
    unzip_entry_source(const std::string& zip_file_path, const std::string& zip_entry_name, file_entry entry) : 
    zip_file_path(std::string(zip_file_path.data(), zip_file_path.length())),
    zip_entry_name(std::string(zip_entry_name.data(), zip_entry_name.length())),
    entry(entry),
    fd(this->zip_file_path),
    inflater(nullptr),
    avail_out(entry.uncomp_length) {
        fd.seek(entry.offset);
        check_header();
        switch (this->entry.comp_method) {
        case static_cast<uint16_t>(sl::compress::zip_compression_method::store): break;
        case static_cast<uint16_t>(sl::compress::zip_compression_method::deflate): inflater.reset(
                new sl::compress::inflate_source<tinydir_file_ref_type>(
                        sl::io::make_reference_source(this->fd)));
            break;
        default: throw unzip_exception(TRACEMSG(
                "Unsupported compression method: [" + sl::support::to_string(this->entry.comp_method) + "],"
                " in entry: [" + this->zip_entry_name + "],"
                " in ZIP file: [" + this->zip_file_path + "]"));
        }
    }
 
    std::streamsize read(sl::io::span<char> span) {
        if (avail_out > 0) {
            size_t len_out = span.size() <= avail_out ? span.size() : avail_out;
            size_t res = read_data(span.data(), len_out);
            avail_out -= res;
            return res > 0 ? res : std::char_traits<char>::eof();
        } 
        return std::char_traits<char>::eof();
    }
 
private:
    void check_header() {
        uint32_t sig = sl::endian::read_32_le<uint32_t>(fd);
        if (zip_cd_start_signature != sig) {
            throw unzip_exception(TRACEMSG(
                    "Cannot find local file header an alleged zip file: [" + zip_file_path + "],"
                    " position: [" + sl::support::to_string(entry.offset) + "]," +
                    " invalid signature: [" + sl::support::to_string(sig) + "]," +
                    " must be: [" + sl::support::to_string(zip_cd_start_signature) + "]"));
        }
        std::array<char, 32> skip;
        sl::io::skip(fd, skip, 22);
        uint16_t namelen = sl::endian::read_16_le<uint16_t>(fd);
        uint16_t exlen = sl::endian::read_16_le<uint16_t>(fd);
        sl::io::skip(fd, skip, namelen + exlen);
    }

    size_t read_data(char* buffer, size_t len_out) {
            switch (entry.comp_method) {
            case static_cast<uint16_t>(sl::compress::zip_compression_method::store): {
                return sl::io::read_all(fd, {buffer, len_out});
            }
            case static_cast<uint16_t>(sl::compress::zip_compression_method::deflate): {
                return sl::io::read_all(*inflater, {buffer, len_out});
            }
            default: throw unzip_exception(TRACEMSG(
                    "Unsupported compression method: [" + sl::support::to_string(entry.comp_method) + "],"
                    " in entry: [" + zip_entry_name + "],"
                    " in ZIP file: [" + zip_file_path + "]"));
            }
    }
};

} // namespace

std::unique_ptr<std::streambuf> open_zip_entry(const file_index& idx, const std::string& entry_name) {
    auto desc = idx.find_zip_entry(entry_name);
    if (-1 == desc.offset) throw unzip_exception(TRACEMSG(
            "Specified zip entry not found: [" + entry_name + "]"));
    try {
        auto src_ptr = new unzip_entry_source{idx.get_zip_file_path(), entry_name, desc};
        auto usrc = sl::io::make_unique_source(src_ptr);
        return std::unique_ptr<std::streambuf>(sl::io::make_unbuffered_istreambuf_ptr(std::move(usrc)));
    } catch (const std::exception& e) {
        throw unzip_exception(TRACEMSG(
                "Error opening zip entry: [" + entry_name + "]" +
                " from zip file: [" + idx.get_zip_file_path() + "]" +
                " with offset: [" + sl::support::to_string(desc.offset) + "]," +
                " length: [" + sl::support::to_string(desc.comp_length) + "]" +
                "\n" + e.what()));
    }
}

} // namespace
}
