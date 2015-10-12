/* 
 * File:   operations.cpp
 * Author: alex
 * 
 * Created on October 11, 2015, 10:21 PM
 */

#include <iostream>

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

const uint32_t ZIP_CD_START_SIGNATURE = 0x04034b50;

class UnzipEntrySource {
    std::string zip_file_path;
    std::string zip_entry_name;
    su::FileDescriptor fd;
    int32_t en_offset; 
    int32_t en_comp_length;
    int32_t en_uncomp_length;
    
    z_stream stream;
    std::array<char, 8192> buf{{}};
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
            // todo: removeme
            throw;
        }
    }
    
    UnzipEntrySource(std::string zip_file_path, std::string zip_entry_name, int32_t offset, 
            int32_t comp_length, int32_t uncomp_length) : 
    zip_file_path(std::move(zip_file_path)),
    zip_entry_name(std::move(zip_entry_name)),
    fd(this->zip_file_path, 'r'),
    en_offset(offset),    
    en_comp_length(comp_length),
    en_uncomp_length(uncomp_length),
    avail_out(uncomp_length) {
        fd.seek(en_offset);
        check_header();
        init_zlib_stream();
    }
    
    std::streamsize read(char* buffer, std::streamsize length) {
        if (0 == avail_out) return std::char_traits<char>::eof();
        // todo: store method
        if(0 == avail) {
            size_t limlen = std::min(buf.size(), en_comp_length - count_in);
            avail = io::read_all(fd, buf.data(), limlen);
            pos = 0;
            count_in += avail;
        }
        stream.next_in = reinterpret_cast<unsigned char*>(buf.data() + pos);
        stream.avail_in = avail;
        stream.next_out = reinterpret_cast<unsigned char*>(buffer);
        auto av_out_pass = std::min(static_cast<size_t> (length), avail_out);
        stream.avail_out = av_out_pass;
        auto ptr = std::addressof(stream);
        auto err = ::inflate(ptr, Z_FINISH);
        if (Z_OK == err || Z_STREAM_END == err || (Z_BUF_ERROR == err && 0 == stream.avail_in)) {
            std::streamsize read = avail - stream.avail_in;
            std::streamsize written = av_out_pass - stream.avail_out;
            pos += read;
            avail -= read;            
            avail_out -= written;
            if (written > 0 || Z_STREAM_END != err) {
                return written;
            }
            return std::char_traits<char>::eof();
        } else throw UnzipException(TRACEMSG(std::string{} +
                "Inflate error: [" + zError(err) + "], file: [" + zip_file_path + "]"));
    }
    
private:
    void check_header() {
        uint32_t sig = io::read_32_le<uint32_t>(fd);
        if (ZIP_CD_START_SIGNATURE != sig) {
            throw UnzipException(TRACEMSG(std::string{} +
            "Cannot find local file header an alleged zip file: [" + zip_file_path + "],"
                    " position: [" + su::to_string(en_offset) + "]," +
                    " invalid signature: [" + su::to_string(sig) + "]," +
                    " must be: [" + su::to_string(ZIP_CD_START_SIGNATURE) + "]"));
        }
        std::array<char, 32> skip;
        io::skip(fd, skip.data(), skip.size(), 22);
        uint16_t namelen = io::read_16_le<uint16_t>(fd);
        uint16_t exlen = io::read_16_le<uint16_t>(fd);
        io::skip(fd, skip.data(), skip.size(), namelen + exlen);
    }
    
    void init_zlib_stream() {
        std::memset(std::addressof(stream), 0, sizeof(stream));
        auto err = inflateInit2(std::addressof(stream), -MAX_WBITS);
        if (Z_OK != err) throw UnzipException(TRACEMSG(std::string{} + 
                "Error initializing ZIP stream: [" + zError(err) + "], file: [" + zip_file_path + "]"));
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
                    new UnzipEntrySource{idx.get_zip_file_path(), entry_name, desc.offset, desc.comp_length, desc.uncomp_length})}};
    } catch (const std::exception& e) {
        throw UnzipException(TRACEMSG(std::string{} + 
                "Error opening zip entry: [" + entry_name + "]" +
                " from zip file: [" + idx.get_zip_file_path() + "]" +
                " with offset: [" + su::to_string(desc.offset) + "], length: [" + su::to_string(desc.comp_length) + "]" +
                "\n" + e.what()));
    }
}

} // namespace
}
