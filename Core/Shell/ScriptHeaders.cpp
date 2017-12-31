/*
 * ScriptHeaders.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/ScriptHeaders.hpp"

const char ScriptHeaders::OBJECT_HEADER[ScriptHeaders::OBJECT_HEADER_SIZE] = {'\x7F', 'B', 'I', 'N'};
const char ScriptHeaders::SCRIPT_HEADER[ScriptHeaders::SCRIPT_HEADER_SIZE] = {'#', '!'};
