/* 
 * File:   UnzipException.hpp
 * Author: alex
 *
 * Created on October 8, 2015, 9:10 PM
 */

#ifndef STATICLIb_UNZIP_UNZIPEXCEPTION_HPP
#define	STATICLIb_UNZIP_UNZIPEXCEPTION_HPP

#include "staticlib/utils/BaseException.hpp"

namespace staticlib {
namespace unzip {

/**
 * Module specific exception
 */
class UnzipException : public staticlib::utils::BaseException {
public:
    /**
     * Default constructor
     */
    UnzipException() = default;

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    UnzipException(const std::string& msg) :
    staticlib::utils::BaseException(msg) { }        

};

} //namespace
}

#endif	/* STATICLIb_UNZIP_UNZIPEXCEPTION_HPP */

