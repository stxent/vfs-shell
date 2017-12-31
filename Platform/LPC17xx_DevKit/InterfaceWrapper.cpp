/*
 * InterfaceWrapper.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "InterfaceWrapper.hpp"

static const InterfaceClass wrapperTable = {
    sizeof(struct InterfaceWrapper),  // size
    InterfaceWrapper::init,           // init
    InterfaceWrapper::deinit,         // deinit

    InterfaceWrapper::setCallback,    // setCallback
    InterfaceWrapper::getParam,       // head
    InterfaceWrapper::setParam,       // free
    InterfaceWrapper::read,           // length
    InterfaceWrapper::write           // next
};

const InterfaceClass * const InterfaceWrapper = &wrapperTable;
