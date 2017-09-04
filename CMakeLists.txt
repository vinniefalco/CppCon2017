#
# Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/boostorg/beast
#

cmake_minimum_required (VERSION 3.0.0)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

source_group ("" FILES main.cpp)

set(CMAKE_CXX_STANDARD 11)

if (MSVC)
    add_definitions (-D_SCL_SECURE_NO_WARNINGS=1)
    add_definitions (-D_CRT_SECURE_NO_WARNINGS=1)
endif()

add_executable (cppcon2017
    main.cpp
)
