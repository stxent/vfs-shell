/*
 * InterfaceProxy.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Interfaces/InterfaceProxy.hpp"

static const InterfaceClass proxyTable = {
    sizeof(struct InterfaceProxy),  // size
    InterfaceProxy::init,           // init
    InterfaceProxy::deinit,         // deinit

    nullptr,                        // setCallback
    InterfaceProxy::getParam,       // getParam
    InterfaceProxy::setParam,       // setParam
    InterfaceProxy::read,           // read
    InterfaceProxy::write           // write
};

const InterfaceClass * const InterfaceProxy = &proxyTable;
