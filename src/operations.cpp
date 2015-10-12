/* 
 * File:   operations.cpp
 * Author: alex
 * 
 * Created on October 11, 2015, 10:21 PM
 */

#include <ios>
#include <string>
#include <array>
#include <cstring>

#include "zlib.h"

#include "staticlib/utils.hpp"
#include "staticlib/io.hpp"

#include "staticlib/unzip/UnzipException.hpp"
#include "staticlib/unzip/UnzipFileIndex.hpp"
#include "staticlib/unzip/operations.hpp"

namespace staticlib {
namespace unzip {

namespace { // anonymous

namespace su = staticlib::utils;
namespace io = staticlib::io;

class UnzipEntrySource {
    std::string zip_file_path;
    std::string zip_entry_name;
    su::FileDescriptor fd;
    int32_t offset; 
    int32_t length;
    
    z_stream stream;
    std::array<char, 8192> buf{{}};
    size_t pos = 0;
    size_t count = 0;
    
public:
    ~UnzipEntrySource() STATICLIB_NOEXCEPT {
        try {
            inflateEnd(std::addressof(stream));
        } catch(...) {
            // ignore
            // todo: removeme
            throw;
        }
    }
    
    UnzipEntrySource(std::string zip_file_path, std::string zip_entry_name, int32_t offset, int32_t length) : 
    zip_file_path(std::move(zip_file_path)),
    zip_entry_name(std::move(zip_entry_name)),
    fd(this->zip_file_path, 'r'),
    offset(offset),
    length(length) {
        fd.seek(offset);
        init_zlib_stream();
    }
    
    std::streamsize read(char* buffer, std::streamsize length) {
        auto avail = io::read_all(fd, buf.data() + pos, std::min(buf.size() - pos, length - count));
        stream.next_in = reinterpret_cast<unsigned char*>(buf.data() + pos);
        stream.avail_in = avail;
        stream.next_out = reinterpret_cast<unsigned char*>(buffer);
        stream.avail_out = length;
        auto err = ::inflate(std::addressof(stream), Z_NO_FLUSH);
        if (err != Z_STREAM_END) {
            inflateEnd(&stream);
            if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
                return Z_DATA_ERROR;
            return err;
        }
        // todo: update pointers
        return length;
    }
    
private:
    void init_zlib_stream() {
        std::memset(std::addressof(stream), 0, sizeof(stream));
        auto err = inflateInit(std::addressof(stream));
        if (Z_OK != err) throw UnzipException(TRACEMSG(std::string{} + 
                "Error initializing ZIP stream: [" + zError(err) + "], file: [" + zip_file_path + "]"));
    }
};

} // namespace

std::unique_ptr<std::streambuf> open_zip_entry_unbuffered(const UnzipFileIndex& idx, const std::string& entry_name) {
    auto desc = idx.find_zip_entry(entry_name);
    if (-1 == desc.first) throw UnzipException(TRACEMSG(std::string{} + 
            "Specified zip entry not found: [" + entry_name + "]"));
    try {
        return std::unique_ptr<std::streambuf>{
            new io::unbuffered_istreambuf<io::unique_source<UnzipEntrySource>>{
                io::make_unique_source(
                    new UnzipEntrySource{idx.get_zip_file_path(), entry_name, desc.first, desc.second})}};
    } catch (const std::exception& e) {
        throw UnzipException(TRACEMSG(std::string{} + 
                "Error opening zip entry: [" + entry_name + "]" +
                " from zip file: [" + idx.get_zip_file_path() + "]" +
                " with offset: [" + su::to_string(desc.first) + "], length: [" + su::to_string(desc.second) + "]" +
                "\n" + e.what()));
    }
}

} // namespace
}
