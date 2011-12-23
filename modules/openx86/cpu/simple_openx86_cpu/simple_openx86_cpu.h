
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
 * @author Eric Borch, Mohit Gambhir
 * @brief Simple CPU for ASIM.
 */

#ifndef _CPU_
#define _CPU_

// generic
#include <iostream>

// ASIM core
#include "asim/syntax.h"
//#include "asim/except.h"
#include "asim/module.h"
#include "asim/cmd.h"
#include "asim/port.h"
#include "asim/trace.h"

// ASIM public modules
#include "asim/provides/isa.h"
#include "asim/provides/context_scheduler.h"
#include "asim/provides/hardware_context.h"
#include "asim/provides/cpu_inst.h"

/*
 * Class ASIM_CPU
 *
 * Simple processor module.
 *
 */
typedef class ASIM_CPU_CLASS *ASIM_CPU;
class ASIM_CPU_CLASS : public ASIM_MODULE_CLASS
{
  private:
  
    // NOTE: only use one of: HW_CONTEXTS or ASIM_THREADS! They are
    // not mutually compatible!
    HW_CONTEXT_CLASS hwc;
    
    IADDR_CLASS nextPc;

    UINT32 chipNum;
    UINT32 cpuNum;

    CPU_INST inst;

    UINT64 instsRetired[NUM_HWCS_PER_CPU];
    
    CPU_INST FetchNextInstruction(const UINT64 cycle);
    void FetchExecuteCommitInstructions(const UINT64 cycle);

  public:
  
    ASIM_CPU_CLASS (ASIM_MODULE parent, const char *name, const UINT32 cpu_num, 
                    const UINT32 chip_number,
                    const UINT32 hwc_priority = 0
                    );

    ~ASIM_CPU_CLASS ();

    /*
     * Initialize the Box
     */
    bool InitModule(void);    

    /*
     * Do a cycle of work...
     */
    void Clock (const UINT64 cycle);
    
     void DumpStats(STATE_OUT state_out, UINT64 statCycles, UINT64 * committed_insn) {};

};

#endif /* _CPU_ */
