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
 * File:   unzip_file_index.cpp
 * Author: alex
 * 
 * Created on October 8, 2015, 8:03 PM
 */

#include "staticlib/unzip/unzip_file_index.hpp"

#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <cstring>

// http://stackoverflow.com/a/1904659/314015
#define NOMINMAX

#include "zlib.h"

#include "staticlib/config.hpp"
#include "staticlib/endian.hpp"
#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

#include "staticlib/unzip/unzip_exception.hpp"


namespace staticlib {
namespace unzip {

namespace { // anonymous

namespace sc = staticlib::config;
namespace en = staticlib::endian;
namespace io = staticlib::io;
namespace su = staticlib::utils;
namespace st = staticlib::tinydir;

const uint32_t zip_cd_start_signature = 0x02014b50;

struct central_directory {
    uint32_t offset;
    uint16_t records_count;
    
    central_directory(uint32_t offset, uint16_t records_count) :
    offset(offset), 
    records_count(records_count) { }
};

struct named_file_entry {
    std::string name;
    uint16_t cd_en_size;
    file_entry entry;

    named_file_entry(std::string name, uint16_t cd_en_size, int32_t offset, int32_t comp_length, 
            int32_t uncomp_length, uint16_t comp_method) :
    name(std::move(name)),
    cd_en_size(cd_en_size),
    entry(offset, comp_length, uncomp_length, comp_method) { }
        
    bool is_file() {
        return '/' != name.back();
    }
    
};

}

class unzip_file_index::impl : public staticlib::pimpl::pimpl_object::impl {
    std::string zip_file_path;
    std::unordered_map<std::string, file_entry> en_map{};
    std::vector<std::string> en_list{};
    
public:
    ~impl() STATICLIB_NOEXCEPT { };
    
    impl(std::string zip_file_path) :
    zip_file_path(std::move(zip_file_path)) {
        auto src = io::make_buffered_source(st::file_source(this->zip_file_path));
        size_t cd_buf_len = std::min(static_cast<size_t>(src.get_source().size()), src.get_buffer().size());
        central_directory cd = find_cd(src.get_source(), src.get_buffer().data(), cd_buf_len);
        src.get_source().seek(cd.offset);
        std::memset(src.get_buffer().data(), 0, src.get_buffer().size());
        for (int i = 0; i < cd.records_count; i++) {
            auto en = read_next_entry(src);
            if (en.is_file()) {
                en_list.push_back(en.name);
                auto res = en_map.emplace(std::move(en.name), en.entry); // value
                if (!res.second) throw unzip_exception(TRACEMSG(
                        "Invalid Duplicate entry: [" + (res.first)->first + "] in a zip file: [" + this->zip_file_path + "]"));
            }
        }
    }

    file_entry find_zip_entry(const unzip_file_index&, const std::string& name) const {
        auto res = en_map.find(name);
        if(res != en_map.end()) {
            return res->second;
        } else {
            return file_entry{};
        }
    }
    
    const std::string& get_zip_file_path(const unzip_file_index&) const {
        return zip_file_path;
    }
    
    const std::vector<std::string>& get_entries(const unzip_file_index&) const {
        return en_list;
    }
    
private:    
    central_directory find_cd(st::file_source& fd, char* buf, std::streamsize buf_size) {
        fd.seek(-buf_size, 'e');
        io::read_exact(fd, {buf, buf_size});
        std::streamsize eocd = -1;
        for (std::streamsize i = buf_size - 1; i >= 3; i--) {
            if (0x06 == buf[i] && 0x05 == buf[i - 1] && 0x4b == buf[i - 2] && 0x50 == buf[i - 3]) {
                eocd = i - 3;
            }
        }
        if (-1 == eocd) throw unzip_exception(TRACEMSG("Cannot find Central Directory" + 
                " in an alleged zip file: [" + zip_file_path + "],"
                " searching through: [" + sc::to_string(buf_size) + "] bytes on the end of the file"));
        if (eocd > buf_size - 22) throw unzip_exception(TRACEMSG("Invalid EOCD position " + 
                " (from end): [" + sc::to_string(buf_size - eocd) + "],"
                " in an alleged zip file: [" + zip_file_path + "]"));
        uint32_t offset; 
        ::memcpy(std::addressof(offset), buf + eocd + 16, 4);
        offset = le32toh(offset);
        uint16_t records_count;
        ::memcpy(std::addressof(records_count), buf + eocd + 8, 2);
        records_count = le16toh(records_count);
        return central_directory(offset, records_count);
    }
  
    named_file_entry read_next_entry(io::buffered_source<st::file_source>& src) {
        uint32_t sig = en::read_32_le<uint32_t>(src);
        if (zip_cd_start_signature != sig) {
            throw unzip_exception(TRACEMSG("Cannot find Central Directory file header" + 
                    " in an alleged zip file: [" + zip_file_path + "]," +
                    " invalid signature: [" + sc::to_string(sig) + "]," +
                    " must be: [" + sc::to_string(zip_cd_start_signature) + "]"));
        }
        std::array<char, 32> skip;
        io::skip(src, skip, 6);
        uint16_t comp_method = en::read_16_le<uint16_t>(src);
        io::skip(src, skip, 8);
        int32_t comp_length = en::read_32_le<int32_t>(src);
        int32_t uncomp_length = en::read_32_le<int32_t>(src);
        uint16_t namelen = en::read_16_le<uint16_t>(src);
        uint16_t extralen = en::read_16_le<uint16_t>(src);
        uint16_t commentlen = en::read_16_le<uint16_t>(src);
        io::skip(src, skip, 8);
        uint32_t offset = en::read_32_le<uint32_t>(src);
        std::string filename{};
        filename.resize(namelen);
        io::read_exact(src, {std::addressof(filename.front()), namelen});
        // skip extra and comment
        io::skip(src, skip, extralen + commentlen);
        return named_file_entry(std::move(filename), 46 + namelen + extralen + commentlen, offset, comp_length, uncomp_length, comp_method);
    }
};
PIMPL_FORWARD_CONSTRUCTOR(unzip_file_index, (std::string), (), unzip_exception)
PIMPL_FORWARD_METHOD(unzip_file_index, file_entry, find_zip_entry, (const std::string&), (const), unzip_exception)
PIMPL_FORWARD_METHOD(unzip_file_index, const std::string&, get_zip_file_path, (), (const), unzip_exception)
PIMPL_FORWARD_METHOD(unzip_file_index, const std::vector<std::string>&, get_entries, (), (const), unzip_exception)

} // namespace
}
