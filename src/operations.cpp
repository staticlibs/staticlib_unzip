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

#include "staticlib/unzip/UnzipException.hpp"


namespace staticlib {
namespace unzip {

namespace { // anonymous

namespace sc = staticlib::config;
namespace su = staticlib::utils;
namespace io = staticlib::io;
namespace en = staticlib::endian;

const uint32_t ZIP_CD_START_SIGNATURE = 0x04034b50;
const uint16_t ZLIB_METHOD_STORE = 0x00;
const uint16_t ZLIB_METHOD_INFLATE = 0x08;

class UnzipEntrySource {
    std::string zip_file_path;
    std::string zip_entry_name;
    su::FileDescriptor fd;
    FileEntry entry; 
    
    z_stream stream;
    std::array<char, 8192> buf;
    size_t pos = 0;
    size_t avail = 0;
    size_t count_in = 0;
    size_t avail_out;
    
public:
    ~UnzipEntrySource() STATICLIB_NOEXCEPT {
        try {
            inflateEnd(std::addressof(stream));
        } catch(...) {
            // ignore
        }
    }
    
    UnzipEntrySource(std::string zip_file_path, std::string zip_entry_name, FileEntry entry) : 
    zip_file_path(std::move(zip_file_path)),
    zip_entry_name(std::move(zip_entry_name)),
    fd(this->zip_file_path, 'r'),
    entry(entry),    
    avail_out(entry.uncomp_length) {
        fd.seek(entry.offset);
        check_header();
        init_zlib_stream();
    }
    
    std::streamsize read(char* buffer, std::streamsize length) {
        if (avail_out > 0) {
            size_t len_out = std::min(static_cast<size_t>(length), avail_out);
            switch (entry.comp_method) {
            case ZLIB_METHOD_STORE: return read_store(buffer, len_out);
            case ZLIB_METHOD_INFLATE: return read_inflate(buffer, len_out);
            default: throw UnzipException(TRACEMSG(std::string{} +
                    "Unsupported compression method: [" + sc::to_string(entry.comp_method) + "],"
                    " in entry: [" + zip_entry_name + "],"
                    " in ZIP file: [" + zip_file_path + "]"));
            }
        } 
        return std::char_traits<char>::eof();
    }
    
private:
    void check_header() {
        uint32_t sig = en::read_32_le<uint32_t>(fd);
        if (ZIP_CD_START_SIGNATURE != sig) {
            throw UnzipException(TRACEMSG(std::string{} +
            "Cannot find local file header an alleged zip file: [" + zip_file_path + "],"
                    " position: [" + sc::to_string(entry.offset) + "]," +
                    " invalid signature: [" + sc::to_string(sig) + "]," +
                    " must be: [" + sc::to_string(ZIP_CD_START_SIGNATURE) + "]"));
        }
        std::array<char, 32> skip;
        io::skip(fd, skip.data(), skip.size(), 22);
        uint16_t namelen = en::read_16_le<uint16_t>(fd);
        uint16_t exlen = en::read_16_le<uint16_t>(fd);
        io::skip(fd, skip.data(), skip.size(), namelen + exlen);
    }
    
    void init_zlib_stream() {
        std::memset(std::addressof(stream), 0, sizeof(stream));
        auto err = inflateInit2(std::addressof(stream), -MAX_WBITS);
        if (Z_OK != err) throw UnzipException(TRACEMSG(std::string{} + 
                "Error initializing ZIP stream: [" + zError(err) + "], file: [" + zip_file_path + "]"));
    }
    
    std::streamsize read_inflate(char* buffer, size_t len_out) {
        // fill buffer if empty
        if (0 == avail) {
            size_t limlen = std::min(buf.size(), entry.comp_length - count_in);
            avail = io::read_all(fd, buf.data(), limlen);
            pos = 0;
            count_in += avail;
        }
        // prepare zlib stream
        stream.next_in = reinterpret_cast<unsigned char*> (buf.data() + pos);
        stream.avail_in = avail;
        stream.next_out = reinterpret_cast<unsigned char*> (buffer);
        stream.avail_out = len_out;
        // call inflate
        auto err = ::inflate(std::addressof(stream), Z_FINISH);
        if (Z_OK == err || Z_STREAM_END == err || Z_BUF_ERROR) {
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
        } else throw UnzipException(TRACEMSG(std::string{}  +
                "Inflate error: [" + zError(err) + "], file: [" + zip_file_path + "]"));
    }
    
    std::streamsize read_store(char* buffer, size_t len_out) {
        size_t res = io::read_all(fd, buffer, len_out);
        avail_out -= res;
        return res > 0 ? res : std::char_traits<char>::eof();
    }
};

} // namespace

std::unique_ptr<std::streambuf> open_zip_entry(const UnzipFileIndex& idx, const std::string& entry_name) {
    auto desc = idx.find_zip_entry(entry_name);
    if (-1 == desc.offset) throw UnzipException(TRACEMSG(std::string{} + 
            "Specified zip entry not found: [" + entry_name + "]"));
    try {
        return std::unique_ptr<std::streambuf>{
            new io::unbuffered_istreambuf<io::unique_source<UnzipEntrySource>>{
                io::make_unique_source(
                    new UnzipEntrySource{idx.get_zip_file_path(), entry_name, desc})}};
    } catch (const std::exception& e) {
        throw UnzipException(TRACEMSG(std::string{} + 
                "Error opening zip entry: [" + entry_name + "]" +
                " from zip file: [" + idx.get_zip_file_path() + "]" +
                " with offset: [" + sc::to_string(desc.offset) + "]," +
                " length: [" + sc::to_string(desc.comp_length) + "]" +
                "\n" + e.what()));
    }
}

} // namespace
}
