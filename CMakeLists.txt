#
# Copyright 2018 Alexander Dahl
#
# SPDX-License-Identifier: GPL-3.0-or-later
# License-Filename: LICENSE
#

cmake_minimum_required(VERSION 3.1)
project(ideal-waddle C)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME}
    main.c
)
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD	99
)
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		_GNU_SOURCE
)
target_link_libraries(${PROJECT_NAME}
	Threads::Threads
)
