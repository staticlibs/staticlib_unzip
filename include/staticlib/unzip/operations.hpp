/* 
 * File:   operations.hpp
 * Author: alex
 *
 * Created on October 8, 2015, 7:12 PM
 */

#ifndef STATICLIB_UNZIP_OPERATIONS_HPP
#define	STATICLIB_UNZIP_OPERATIONS_HPP

#include <memory>
#include <streambuf>

#include "staticlib/unzip/UnzipFileIndex.hpp"

namespace staticlib {
namespace unzip {

std::unique_ptr<std::streambuf> open_zip_entry(const UnzipFileIndex& idx, const std::string& entry_name);

void test_direct();

void test_mini();

} // namespace
}

#endif	/* STATICLIB_UNZIP_OPERATIONS_HPP */

