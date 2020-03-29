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

# project
project ( staticlib_unzip CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 1.0.8 )
set ( ${PROJECT_NAME}_DESCRIPTION "Staticlib unZIP library" )
set ( ${PROJECT_NAME}_URL https://github.com/staticlibs/staticlib_unzip )
include ( ${CMAKE_CURRENT_LIST_DIR}/resources/macros.cmake )

# docs
option ( ${PROJECT_NAME}_ENABLE_DOCS "Generate doxyfile and exit build" OFF )
if ( ${PROJECT_NAME}_ENABLE_DOCS )
    configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
    return ( )
endif ( )

# check deplibs cache
if ( STATICLIB_USE_DEPLIBS_CACHE )
    set ( ${PROJECT_NAME}_CACHED_LIB_PATH ${STATICLIB_DEPLIBS_CACHE_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX} )
    if ( EXISTS ${${PROJECT_NAME}_CACHED_LIB_PATH} )
        add_library( ${PROJECT_NAME} STATIC IMPORTED GLOBAL )
        set_target_properties( ${PROJECT_NAME} PROPERTIES IMPORTED_LOCATION ${${PROJECT_NAME}_CACHED_LIB_PATH} )
        message ( STATUS "Using cached library: [${${PROJECT_NAME}_CACHED_LIB_PATH}]" )
        return ( )
    endif ( )
endif ( )

# standalone build
if ( NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY )
    set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
endif ( )
if ( NOT DEFINED STATICLIB_TOOLCHAIN )
    if ( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" ) 
        staticlib_unzip_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../external_zlib )
    endif (  )
    staticlib_unzip_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_config )
    staticlib_unzip_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_support )
    staticlib_unzip_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_io )
    staticlib_unzip_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_endian )
    staticlib_unzip_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_compress )
    staticlib_unzip_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_pimpl )
    staticlib_unzip_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_utils )
    staticlib_unzip_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_tinydir )
endif ( )

# dependencies
set ( ${PROJECT_NAME}_OPTIONS "" )
set ( ${PROJECT_NAME}_DEPS_PUBLIC
        staticlib_config
        staticlib_support
        staticlib_endian
        staticlib_io
        staticlib_compress
        staticlib_pimpl
        staticlib_utils 
        staticlib_tinydir )
set ( ${PROJECT_NAME}_DEPS_PRIVATE
        zlib )
set ( ${PROJECT_NAME}_DEPS ${${PROJECT_NAME}_DEPS_PUBLIC} ${${PROJECT_NAME}_DEPS_PRIVATE} )
staticlib_unzip_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

# build steps
file ( GLOB_RECURSE ${PROJECT_NAME}_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp )
file ( GLOB_RECURSE ${PROJECT_NAME}_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp )
add_library ( ${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRC} ${${PROJECT_NAME}_HEADERS} )
source_group ( "src" FILES ${${PROJECT_NAME}_SRC} )
source_group ( "include" FILES ${${PROJECT_NAME}_HEADERS} )
set ( ${PROJECT_NAME}_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/include )
target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE
        ${${PROJECT_NAME}_INCLUDES}         
        ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS} )
target_compile_options ( ${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_OPTIONS} ${${PROJECT_NAME}_DEPS_PC_CFLAGS_OTHER} )
if ( TARGET external_zlib )
    add_dependencies ( ${PROJECT_NAME} external_zlib )
endif ( )

# pkg-config
staticlib_unzip_list_to_string ( ${PROJECT_NAME}_PC_CFLAGS_INCLUDES "-I" ${PROJECT_NAME}_INCLUDES )
staticlib_unzip_list_to_string ( ${PROJECT_NAME}_PC_CFLAGS_OPTS "" ${PROJECT_NAME}_OPTIONS )
set ( ${PROJECT_NAME}_PC_CFLAGS "${${PROJECT_NAME}_PC_CFLAGS_INCLUDES} ${${PROJECT_NAME}_PC_CFLAGS_OPTS}" )
set ( ${PROJECT_NAME}_PC_LIBS "-L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -l${PROJECT_NAME}" )
staticlib_unzip_list_to_string ( ${PROJECT_NAME}_PC_REQUIRES "" ${PROJECT_NAME}_DEPS_PUBLIC )
staticlib_unzip_list_to_string ( ${PROJECT_NAME}_PC_REQUIRES_PRIVATE "" ${PROJECT_NAME}_DEPS_PRIVATE )
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/pkg-config.in 
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pkgconfig/${PROJECT_NAME}.pc )
