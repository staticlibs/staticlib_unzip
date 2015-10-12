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
    
    std::pair<int32_t, int32_t> find_zip_entry(const std::string& name) const;
    
    const std::string& get_zip_file_path() const;
};

} // namespace
}

#endif	/* STATICLIB_UNZIP_UNZIPFILEINDEX_HPP */

