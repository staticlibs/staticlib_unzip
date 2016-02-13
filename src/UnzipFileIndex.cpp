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
 * File:   UnzipFileIndex.cpp
 * Author: alex
 * 
 * Created on October 8, 2015, 8:03 PM
 */

#include "staticlib/unzip/UnzipFileIndex.hpp"

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
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

#include "staticlib/unzip/UnzipException.hpp"


namespace staticlib {
namespace unzip {

namespace { // anonymous

namespace sc = staticlib::config;
namespace en = staticlib::endian;
namespace io = staticlib::io;
namespace su = staticlib::utils;

const uint32_t ZIP_CD_START_SIGNATURE = 0x02014b50;

#ifdef STATICLIB_WITH_ICU
std::string to_utf8(const icu::UnicodeString& str) {
    std::string bytes;
    icu::StringByteSink<std::string> sbs(&bytes);
    str.toUTF8(sbs);
    return bytes;
}
#endif

struct CentralDirectory {
    uint32_t offset;
    uint16_t records_count;
    
    CentralDirectory(uint32_t offset, uint16_t records_count) :
    offset(offset), 
    records_count(records_count) { }
};

struct NamedFileEntry {
    std::string name;
    uint16_t cd_en_size;
    FileEntry entry;

    NamedFileEntry(std::string name, uint16_t cd_en_size, int32_t offset, int32_t comp_length, 
            int32_t uncomp_length, uint16_t comp_method) :
    name(std::move(name)),
    cd_en_size(cd_en_size),
    entry(offset, comp_length, uncomp_length, comp_method) { }
        
    bool is_file() {
        return '/' != name.back();
    }
    
};

}

class UnzipFileIndex::Impl : public staticlib::pimpl::PimplObject::Impl {
    std::string zip_file_path;
#ifdef STATICLIB_WITH_ICU
    icu::UnicodeString zip_file_upath;
#endif // STATICLIB_WITH_ICU
    std::unordered_map<std::string, FileEntry> en_map{};
    std::vector<std::string> en_list{};
    
public:
    ~Impl() STATICLIB_NOEXCEPT { };
    
#ifdef STATICLIB_WITH_ICU
    Impl(icu::UnicodeString zip_file_upath) :
    zip_file_path(to_utf8(zip_file_upath)),
    zip_file_upath(std::move(zip_file_upath)) {
        io::buffered_source<su::FileDescriptor> src{su::FileDescriptor{this->zip_file_upath, 'r'}};
#else    
    Impl(std::string zip_file_path) :
    zip_file_path(std::move(zip_file_path)) {
        io::buffered_source<su::FileDescriptor> src{su::FileDescriptor{this->zip_file_path, 'r'}};
#endif // STATICLIB_WITH_ICU        
        size_t cd_buf_len = std::min(static_cast<size_t>(src.get_source().size()), src.get_buffer().size());
        CentralDirectory cd = find_cd(src.get_source(), src.get_buffer().data(), cd_buf_len);
        src.get_source().seek(cd.offset);
        std::memset(src.get_buffer().data(), 0, src.get_buffer().size());
        for (int i = 0; i < cd.records_count; i++) {
            auto en = read_next_entry(src);
            if (en.is_file()) {
                en_list.push_back(en.name);
                auto res = en_map.emplace(std::move(en.name), en.entry); // value
                if (!res.second) throw UnzipException(TRACEMSG(std::string{} +
                        "Invalid Duplicate entry: [" + (res.first)->first + "] in a zip file: [" + this->zip_file_path + "]"));
            }
        }
    }

#ifdef STATICLIB_WITH_ICU
    FileEntry find_zip_entry(const UnzipFileIndex&, const icu::UnicodeString& uname) const {
        std::string name = to_utf8(uname);
#else    
    FileEntry find_zip_entry(const UnzipFileIndex&, const std::string& name) const {
#endif // STATICLIB_WITH_ICU        
        auto res = en_map.find(name);
        if(res != en_map.end()) {
            return res->second;
        } else {
            return FileEntry{};
        }
    }
    
    const std::string& get_zip_file_path(const UnzipFileIndex&) const {
        return zip_file_path;
    }
    
#ifdef STATICLIB_WITH_ICU
    const icu::UnicodeString& get_zip_file_upath(const UnzipFileIndex&) const {
        return zip_file_upath;
    }
#endif // STATICLIB_WITH_ICU     
    
    const std::vector<std::string>& get_entries(const UnzipFileIndex&) const {
        return en_list;
    }
    
private:    
    CentralDirectory find_cd(su::FileDescriptor& fd, char* buf, std::streamsize buf_size) {
        fd.seek(-buf_size, 'e');
        io::read_exact(fd, buf, buf_size);
        std::streamsize eocd = -1;
        for (std::streamsize i = buf_size - 1; i >= 3; i--) {
            if (0x06 == buf[i] && 0x05 == buf[i - 1] && 0x4b == buf[i - 2] && 0x50 == buf[i - 3]) {
                eocd = i - 3;
            }
        }
        if (-1 == eocd) throw UnzipException(TRACEMSG(std::string{} +
                "Cannot find Central Directory in an alleged zip file: [" + zip_file_path + "],"
                " searching through: [" + sc::to_string(buf_size) + "] bytes on the end of the file"));
        if (eocd > buf_size - 22) throw UnzipException(TRACEMSG(std::string{} +
                "Invalid EOCD position (from end): [" + sc::to_string(buf_size - eocd) + "],"
                " in an alleged zip file: [" + zip_file_path + "]"));
        uint32_t offset; 
        ::memcpy(std::addressof(offset), buf + eocd + 16, 4);
        offset = le32toh(offset);
        uint16_t records_count;
        ::memcpy(std::addressof(records_count), buf + eocd + 8, 2);
        records_count = le16toh(records_count);
        return CentralDirectory(offset, records_count);
    }
  
    NamedFileEntry read_next_entry(io::buffered_source<su::FileDescriptor>& src) {
        uint32_t sig = en::read_32_le<uint32_t>(src);
        if (ZIP_CD_START_SIGNATURE != sig) {
            throw UnzipException(TRACEMSG(std::string{} +
                    "Cannot find Central Directory file header in an alleged zip file: [" + zip_file_path + "]," +
                    " invalid signature: [" + sc::to_string(sig) + "]," +
                    " must be: [" + sc::to_string(ZIP_CD_START_SIGNATURE) + "]"));
        }
        std::array<char, 32> skip;
        io::skip(src, skip.data(), skip.size(), 6);
        uint16_t comp_method = en::read_16_le<uint16_t>(src);
        io::skip(src, skip.data(), skip.size(), 8);
        int32_t comp_length = en::read_32_le<int32_t>(src);
        int32_t uncomp_length = en::read_32_le<int32_t>(src);
        uint16_t namelen = en::read_16_le<uint16_t>(src);
        uint16_t extralen = en::read_16_le<uint16_t>(src);
        uint16_t commentlen = en::read_16_le<uint16_t>(src);
        io::skip(src, skip.data(), skip.size(), 8);
        uint32_t offset = en::read_32_le<uint32_t>(src);
        std::string filename{};
        filename.resize(namelen);
        io::read_exact(src, std::addressof(filename.front()), namelen);
        // skip extra and comment
        io::skip(src, skip.data(), skip.size(), extralen + commentlen);
        return NamedFileEntry(std::move(filename), 46 + namelen + extralen + commentlen, offset, comp_length, uncomp_length, comp_method);
    }
};
#ifdef STATICLIB_WITH_ICU
PIMPL_FORWARD_CONSTRUCTOR(UnzipFileIndex, (icu::UnicodeString), (), UnzipException)
PIMPL_FORWARD_METHOD(UnzipFileIndex, FileEntry, find_zip_entry, (const icu::UnicodeString&), (const), UnzipException)
PIMPL_FORWARD_METHOD(UnzipFileIndex, const std::string&, get_zip_file_path, (), (const), UnzipException)
PIMPL_FORWARD_METHOD(UnzipFileIndex, const icu::UnicodeString&, get_zip_file_upath, (), (const), UnzipException)
#else
PIMPL_FORWARD_CONSTRUCTOR(UnzipFileIndex, (std::string), (), UnzipException)
PIMPL_FORWARD_METHOD(UnzipFileIndex, FileEntry, find_zip_entry, (const std::string&), (const), UnzipException)
PIMPL_FORWARD_METHOD(UnzipFileIndex, const std::string&, get_zip_file_path, (), (const), UnzipException)
#endif // STATICLIB_WITH_ICU
PIMPL_FORWARD_METHOD(UnzipFileIndex, const std::vector<std::string>&, get_entries, (), (const), UnzipException)

} // namespace
}
