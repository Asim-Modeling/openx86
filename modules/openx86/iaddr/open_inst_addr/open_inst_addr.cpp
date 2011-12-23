/**************************************************************************
 * Copyright (C) 2011 Intel Corporation
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **************************************************************************/

/**
 * @file
 * @author Chris Weaver and Mark Charney
 *
 * @brief This file contains information about X86 instruction addresses.
 *
 */

// generic
#include <ostream>
#include <iomanip>

// ASIM public modules
#include "asim/provides/iaddr.h"

using namespace std;

/// Output operator for instruction address.
ostream &
operator << (ostream & os,const IADDR_CLASS & ia)
{
    os << "LIP:0x"<< fmt_x(ia.Get());
    if (ia.GetMicro() != 0)
    {
        os << "/0x"<< fmt_x(ia.GetMicro());
    }
    return os;
}

