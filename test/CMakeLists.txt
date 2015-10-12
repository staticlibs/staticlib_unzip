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

# toolchain setup
set ( STATICLIB_TOOLCHAIN linux_amd64_gcc CACHE STRING "toolchain triplet" )
if ( NOT DEFINED STATICLIB_CMAKE )
    set ( STATICLIB_CMAKE ${CMAKE_CURRENT_LIST_DIR}/../cmake CACHE INTERNAL "" )
endif ( )
set ( CMAKE_TOOLCHAIN_FILE ${STATICLIB_CMAKE}/toolchains/${STATICLIB_TOOLCHAIN}.cmake CACHE INTERNAL "" )

# project
project ( staticlib_unzip_test CXX )
include ( ${STATICLIB_CMAKE}/staticlibs_common.cmake )

# dependencies
if ( NOT STATICLIB_TOOLCHAIN MATCHES "linux_[^_]+_[^_]+" )
    staticlib_add_subdirectory ( ../deps external_zlib )
endif ( )
staticlib_add_subdirectory ( ../deps staticlib_utils )
staticlib_add_subdirectory ( ../deps staticlib_pimpl )
staticlib_add_subdirectory ( ../deps staticlib_io )
staticlib_add_subdirectory ( ../.. staticlib_unzip )
set ( ${PROJECT_NAME}_DEP_MODULES staticlib_unzip staticlib_pimpl )     
set ( ${PROJECT_NAME}_DEP_MODULES_PRIVATE zlib )     
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEP_PC REQUIRED ${PROJECT_NAME}_DEP_MODULES )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEP_PC_PRIVATE REQUIRED ${PROJECT_NAME}_DEP_MODULES_PRIVATE )

# tests
set ( ${PROJECT_NAME}_TEST_INCLUDES ${${PROJECT_NAME}_DEP_PC_INCLUDE_DIRS} )
set ( ${PROJECT_NAME}_TEST_LIBS ${${PROJECT_NAME}_DEP_PC_LIBRARIES} ${${PROJECT_NAME}_DEP_PC_PRIVATE_LIBRARIES} )
set ( ${PROJECT_NAME}_TEST_OPTS ${${PROJECT_NAME}_DEP_PC_CFLAGS_OTHER} )
staticlib_enable_testing ( ${PROJECT_NAME}_TEST_INCLUDES ${PROJECT_NAME}_TEST_LIBS ${PROJECT_NAME}_TEST_OPTS )
