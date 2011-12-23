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
 * @file open_x86_regs_none.h
 * @author Michael Adler
 *
 */

#ifndef _OPEN_X86_REGS_NONE_
#define _OPEN_X86_REGS_NONE_

// ASIM core
#include "asim/arch_register.h"

//
// NULL implementation of the register descriptor class for models that
// don't share register names with feeders.
//

class REGISTER_NAMES_CLASS
{
  public:
    // ---------------------------------------------------------------------
    //
    // All REGISTER_NAMES_CLASSes must have exactly the following public
    //  interface.  We could enforce it with virtual functions but the
    //  feeder uses values too often.
    //
    // ---------------------------------------------------------------------

    REGISTER_NAMES_CLASS() {};
    ~REGISTER_NAMES_CLASS() {};

    //
    // ArchRegType -- What is Asim's type for the reigster?
    //
    ARCH_REGISTER_TYPE ArchRegType(UINT32 rNum) const { return REG_TYPE_INVALID; };

    //
    // IsArchReg -- Is the register architecturally visible?
    //
    bool IsArchReg(UINT32 rNum) const { return false; };

    //
    // IsUarchReg -- Is the register a microarchitectural register?
    //    IsArchReg and IsUarchReg are mutually exclusive, though it is
    //    possible for both to be false.
    //
    bool IsUarchReg(UINT32 rNum) const { return false; };

    //
    // HasParent -- Is the register contained inside another register?
    //    E.g. The parent of EAX is RAX.
    //
    bool HasParent(UINT32 rNum) const { return false; };

    //
    // Parent -- Return the number of the parent register (see HasParent
    //     above).  Returns the input register number for registers that
    //     have no parents.
    //
    UINT32 Parent(UINT32 rNum) const { return rNum; };

    //
    // Name -- Guess!
    //
    const char *Name(UINT32 rNum) const { return 0; };

    //
    // SoftSDVName -- The name of the register in SoftSDV's namespace.
    //
    const char *SoftSDVName(UINT32 rNum) const { return 0; };

    //
    // Size -- Register size (bytes)
    //
    UINT32 Size(UINT32 rNum) const { return 0; };

    //
    // NumRegs -- Number of registers.
    //
    UINT32 NumRegs(void) const { return 0; };
};

#endif  // _OPEN_X86_REGS_NONE_
