/* 
 * File:   UnzipFileIndex.hpp
 * Author: alex
 *
 * Created on October 8, 2015, 6:58 PM
 */

#ifndef STATICLIB_UNZIP_UNZIPFILEINDEX_HPP
#define	STATICLIB_UNZIP_UNZIPFILEINDEX_HPP

#include <string>
#include <utility>
#include <cstdint>

#include "staticlib/pimpl.hpp"

namespace staticlib {
namespace unzip {

struct FileEntry {
    int32_t offset = -1;
    int32_t comp_length = -1;
    int32_t uncomp_length = -1;
    uint16_t comp_method = 0;

    FileEntry() { }
    
    FileEntry(int32_t offset, int32_t comp_length, int32_t uncomp_length, uint16_t comp_method) :
    offset(offset),
    comp_length(comp_length),
    uncomp_length(uncomp_length),
    comp_method(comp_method) { }
    
    bool is_empty() {
        return -1 == offset;
    }
};

class UnzipFileIndex : public staticlib::pimpl::PimplObject {
protected:
    /**
     * Implementation class
     */
    class Impl;
public:
    /**
     * PIMPL-specific constructor
     * 
     * @param pimpl impl object
     */
    PIMPL_CONSTRUCTOR(UnzipFileIndex)
            
    UnzipFileIndex(std::string zip_file_path);
    
    FileEntry find_zip_entry(const std::string& name) const;
    
    const std::string& get_zip_file_path() const;
};

} // namespace
}

#endif	/* STATICLIB_UNZIP_UNZIPFILEINDEX_HPP */

