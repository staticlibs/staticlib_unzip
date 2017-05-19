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

#include <ios>
#include <string>
#include <array>
#include <algorithm>
#include <cstring>

#include "zlib.h"

// http://stackoverflow.com/a/1904659/314015
#define NOMINMAX

#include "staticlib/config.hpp"
#include "staticlib/endian.hpp"
#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"
#include "staticlib/tinydir.hpp"

#include "staticlib/unzip/unzip_exception.hpp"


namespace staticlib {
namespace unzip {

namespace { // anonymous

const uint32_t zip_cd_start_signature = 0x04034b50;
const uint16_t zlib_method_store = 0x00;
const uint16_t zib_method_inflate = 0x08;

class unzip_entry_source {
    std::string zip_file_path;
    std::string zip_entry_name;
    sl::tinydir::file_source fd;
    file_entry entry; 
    
    z_stream stream;
    std::array<char, 8192> buf;
    size_t pos = 0;
    size_t avail = 0;
    size_t count_in = 0;
    size_t avail_out;
    
public:
    ~unzip_entry_source() STATICLIB_NOEXCEPT {
        try {
            inflateEnd(std::addressof(stream));
        } catch(...) {
            // ignore
        }
    }

    unzip_entry_source(std::string zip_file_path, std::string zip_entry_name, file_entry entry) : 
    zip_file_path(std::move(zip_file_path)),
    zip_entry_name(std::move(zip_entry_name)),
    fd(this->zip_file_path),
    entry(entry),    
    avail_out(entry.uncomp_length) {
        fd.seek(entry.offset);
        check_header();
        init_zlib_stream();
    }
    
    std::streamsize read(sl::io::span<char> span) {
        if (avail_out > 0) {
            size_t len_out = span.size() <= avail_out ? span.size() : avail_out;
            switch (entry.comp_method) {
            case zlib_method_store: return read_store(span.data(), len_out);
            case zib_method_inflate: return read_inflate(span.data(), len_out);
            default: throw unzip_exception(TRACEMSG(
                    "Unsupported compression method: [" + sl::support::to_string(entry.comp_method) + "],"
                    " in entry: [" + zip_entry_name + "],"
                    " in ZIP file: [" + zip_file_path + "]"));
            }
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
    
    void init_zlib_stream() {
        std::memset(std::addressof(stream), 0, sizeof(stream));
        auto err = inflateInit2(std::addressof(stream), -MAX_WBITS);
        if (Z_OK != err) throw unzip_exception(TRACEMSG(
                "Error initializing ZIP stream: [" + zError(err) + "], file: [" + zip_file_path + "]"));
    }
    
    std::streamsize read_inflate(char* buffer, size_t len_out) {
        // fill buffer if empty
        if (0 == avail) {
            size_t limlen = std::min(buf.size(), entry.comp_length - count_in);
            avail = sl::io::read_all(fd, {buf.data(), limlen});
            pos = 0;
            count_in += avail;
        }
        // prepare zlib stream
        stream.next_in = reinterpret_cast<unsigned char*> (buf.data() + pos);
        stream.avail_in = static_cast<uInt>(avail);
        stream.next_out = reinterpret_cast<unsigned char*> (buffer);
        stream.avail_out = static_cast<uInt>(len_out);
        // call inflate
        auto err = ::inflate(std::addressof(stream), Z_FINISH);
        if (Z_OK == err || Z_STREAM_END == err || Z_BUF_ERROR == err) {
            std::streamsize read = avail - stream.avail_in;
            std::streamsize written = len_out - stream.avail_out;
            size_t uread = static_cast<size_t>(read);
            pos += uread;
            avail -= uread;
            avail_out -= static_cast<size_t>(written);
            if (written > 0 || Z_STREAM_END != err) {
                return written;
            }
            return std::char_traits<char>::eof();
        } else throw unzip_exception(TRACEMSG(
                "Inflate error: [" + zError(err) + "], file: [" + zip_file_path + "]"));
    }
    
    std::streamsize read_store(char* buffer, size_t len_out) {
        size_t res = sl::io::read_all(fd, {buffer, len_out});
        avail_out -= res;
        return res > 0 ? res : std::char_traits<char>::eof();
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
