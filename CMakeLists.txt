#
# Copyright 2018 Alexander Dahl
#
# SPDX-License-Identifier: GPL-3.0-or-later
# License-Filename: LICENSE
#

cmake_minimum_required(VERSION 3.1)
project(ideal-waddle C)

add_executable(${PROJECT_NAME}
    main.c
    list.c
)
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD	99
)
