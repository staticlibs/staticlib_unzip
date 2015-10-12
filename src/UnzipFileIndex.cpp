/* 
 * File:   UnzipFileIndex.cpp
 * Author: alex
 * 
 * Created on October 8, 2015, 8:03 PM
 */

#include <iostream>

#include <string>
#include <unordered_map>
#include <utility>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "zlib.h"

#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"
#include "staticlib/pimpl.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

#include "staticlib/unzip/UnzipException.hpp"
#include "staticlib/unzip/UnzipFileIndex.hpp"

namespace staticlib {
namespace unzip {

namespace { // anonymous

namespace su = staticlib::utils;
namespace io = staticlib::io;

const uint32_t ZIP_CD_START_SIGNATURE = 0x02014b50;

struct CentralDirectory {
    uint32_t offset;
    uint16_t records_count;
    
    CentralDirectory(std::streamsize offset, std::streamsize records_count) :
    offset(offset), 
    records_count(records_count) { }
};

struct NamedFileEntry {
    std::string name;
    FileEntry entry;

    NamedFileEntry(std::string name, int32_t offset, int32_t comp_length, int32_t uncomp_length) :
    name(std::move(name)),
    entry(offset, comp_length, uncomp_length) { }
        
    bool is_file() {
        return '/' != name.back();
    }
    
    int32_t get_offset() {
        return entry.offset;
    }

    int32_t get_comp_length() {
        return entry.comp_length;
    }

    int32_t get_uncomp_length() {
        return entry.uncomp_length;
    }
};

}

class UnzipFileIndex::Impl : public staticlib::pimpl::PimplObject::Impl {
    std::string zip_file_path;
    std::unordered_map<std::string, FileEntry> en_map{};
    
public:
    Impl(std::string zip_file_path) :
    zip_file_path(std::move(zip_file_path)) {
        // creating buffered source early to reuse buffer
        io::buffered_source<su::FileDescriptor> src{su::FileDescriptor{this->zip_file_path, 'r'}};
        size_t cd_buf_len = std::min(static_cast<size_t>(src.get_source().size()), src.get_buffer().size());
        CentralDirectory cd = find_cd(src.get_source(), src.get_buffer().data(), cd_buf_len);
        src.get_source().seek(cd.offset);
        std::memset(src.get_buffer().data(), 0, src.get_buffer().size());
        for (int i = 0; i < cd.records_count; i++) {
            auto en = read_next_entry(src);
            if (en.is_file()) {
                auto res = en_map.emplace(std::piecewise_construct, 
                        std::forward_as_tuple(std::move(en.name)), // key
                        std::forward_as_tuple(en.get_offset(), en.get_comp_length(), en.get_uncomp_length())); // value
                if (!res.second) throw UnzipException(TRACEMSG(std::string{} +
                        "Invalid Duplicate entry: [" + (res.first)->first + "] in a zip file: [" + this->zip_file_path + "]"));
            }
        }
    }
    
    FileEntry find_zip_entry(const UnzipFileIndex&, const std::string& name) const {
        auto res = en_map.find(name);
        if(res != en_map.end()) {
            return res->second;
        } else {
            return {-1, -1, -1};
        }
    }
    
    const std::string& get_zip_file_path(const UnzipFileIndex&) const {
        return zip_file_path;
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
                " searching through: [" + su::to_string(buf_size) + "] bytes on the end of the file"));
        if (eocd > buf_size - 22) throw UnzipException(TRACEMSG(std::string{} +
                "Invalid EOCD position (from end): [" + su::to_string(buf_size - eocd) + "],"
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
        uint32_t sig = io::read_32_le<uint32_t>(src);
        if (ZIP_CD_START_SIGNATURE != sig) {
            throw UnzipException(TRACEMSG(std::string{} +
                    "Cannot find Central Directory file header an alleged zip file: [" + zip_file_path + "]," +
                    " invalid signature: [" + su::to_string(sig) + "]," +
                    " must be: [" + su::to_string(ZIP_CD_START_SIGNATURE) + "]"));
        }
        std::array<char, 32> skip;
        io::skip(src, skip.data(), skip.size(), 16);
        int32_t comp_length = io::read_32_le<int32_t>(src);
        int32_t uncomp_length = io::read_32_le<int32_t>(src);
        uint16_t namelen = io::read_16_le<uint16_t>(src);
        uint16_t extralen = io::read_16_le<uint16_t>(src);
        uint16_t commentlen = io::read_16_le<uint16_t>(src);
        io::skip(src, skip.data(), skip.size(), 8);
        uint32_t offset = io::read_32_le<uint32_t>(src);
        std::string filename{};
        filename.resize(namelen);
        io::read_exact(src, std::addressof(filename.front()), namelen);
        // skip extra and comment
        io::skip(src, skip.data(), skip.size(), extralen + commentlen);
        return NamedFileEntry(std::move(filename), offset, comp_length, uncomp_length);
    }
};
PIMPL_FORWARD_CONSTRUCTOR(UnzipFileIndex, (std::string), (), UnzipException)
PIMPL_FORWARD_METHOD(UnzipFileIndex, FileEntry, find_zip_entry, (const std::string&), (const), UnzipException)
PIMPL_FORWARD_METHOD(UnzipFileIndex, const std::string&, get_zip_file_path, (), (const), UnzipException)        

} // namespace
}
