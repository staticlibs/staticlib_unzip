# Copyright 2015, alex at staticlibs.net
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required ( VERSION 2.8.12 )
include ( ${CMAKE_CURRENT_LIST_DIR}/resources/macros.cmake )

# project
project ( staticlib_unzip CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 1.0.0 )
set ( ${PROJECT_NAME}_DESCRIPTION "Staticlib unZIP library" )
set ( ${PROJECT_NAME}_URL https://github.com/staticlibs/staticlib_unzip )

# docs
option ( ${PROJECT_NAME}_ENABLE_DOCS "Generate doxyfile and exit build" OFF )
if ( ${PROJECT_NAME}_ENABLE_DOCS )
    configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
    return ( )
endif ( )

# dependencies
set ( ${PROJECT_NAME}_DEP_MODULES
        staticlib_utils
        staticlib_pimpl
        staticlib_io )
set ( ${PROJECT_NAME}_DEP_PRIVATE_MODULES
        zlib )
set ( ${PROJECT_NAME}_DEP_ALL_MODULES ${${PROJECT_NAME}_DEP_MODULES} ${${PROJECT_NAME}_DEP_PRIVATE_MODULES} )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEP_PC REQUIRED ${PROJECT_NAME}_DEP_ALL_MODULES )

# build steps
file ( GLOB_RECURSE ${PROJECT_NAME}_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp )
add_library ( ${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRC} )
set ( ${PROJECT_NAME}_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/include )
target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE
        ${${PROJECT_NAME}_INCLUDES}         
        ${${PROJECT_NAME}_DEP_PC_INCLUDE_DIRS} )
target_compile_options ( ${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEP_PC_CFLAGS_OTHER} )

# pkg-config
if ( NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY )
    set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
endif ( )
staticlib_list_to_string ( ${PROJECT_NAME}_PC_CFLAGS "-I" ${PROJECT_NAME}_INCLUDES )
set ( ${PROJECT_NAME}_PC_LIBS "-L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -l${PROJECT_NAME}" )
staticlib_list_to_string ( ${PROJECT_NAME}_PC_REQUIRES "" ${PROJECT_NAME}_DEP_MODULES )
staticlib_list_to_string ( ${PROJECT_NAME}_PC_REQUIRES_PRIVATE "" ${PROJECT_NAME}_DEP_PRIVATE_MODULES )
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/pkg-config.in 
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pkgconfig/${PROJECT_NAME}.pc )
