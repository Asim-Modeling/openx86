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
 * @author Mohit Gambhir (mohit.gambhir@intel.com)
 * @brief Simple board class implementation
 */

// Generic C++
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <list>

#include "asim/syntax.h"
#include "asim/provides/board.h"
#include "asim/provides/chip.h"

/**
 * Instantiate module and submodules.
 */
ASIM_BOARD_CLASS::ASIM_BOARD_CLASS (
    ASIM_MODULE parent, ///< parent module
    const char * const name)   ///< name of this module
    : ASIM_MODULE_CLASS(parent, name)
{

    ASSERT((NUM_CHIPS & (NUM_CHIPS - 1)) == 0, "Number of chips must be power of 2");

    myChips = new ASIM_CHIP [NUM_CHIPS];

    // New each submodule
    for (UINT32 i = 0; i < NUM_CHIPS; i++)
    {
        ostringstream name;
        name << "CHIP_" << i;
        myChips[i] = new ASIM_CHIP_CLASS(this, name.str().c_str(), i);
    }    

}

/**
 * Destroy module and submodules
 */
ASIM_BOARD_CLASS::~ASIM_BOARD_CLASS ()
{
    // Delete submodules
    for (UINT32 i=0; i<NUM_CHIPS; ++i)
    {
        delete myChips[i];
    }    
    delete [] myChips;
}

/**
 * Initialize module and submodules
 */
bool
ASIM_BOARD_CLASS::InitModule ()
{
    bool ok = true;

    TRACE(Trace_Debug, cout << "0: Init Module " << Name() << endl);

    for (UINT32 i=0; i<NUM_CHIPS; ++i)
    {
        ok &= myChips[i]->InitModule();
    }

    if ( !ok ) 
    {
        cout << Name() << ": InitModule failed" << endl;
    }

    return ok;
}

/**
 * Perform one cycle of work.
 */
void
ASIM_BOARD_CLASS::Clock (
    UINT64 cycle)        ///< current cycle
{
    
    ASSERT(false, "Board should not be clocked by now!!");
    
    TRACE(Trace_Sys, cout << cycle << ": Clocking - " << Name() << endl);

    for (UINT32 i=0; i<NUM_CHIPS; ++i)
    {
        myChips[i]->Clock(cycle);
    }
}

void
ASIM_BOARD_CLASS::DumpStats (
    STATE_OUT state_out, UINT64 statCycles, UINT64 * committed_inst)
{
    for (UINT16 i=0; i<NUM_CHIPS; ++i)
    {
        myChips[i]->DumpStats(
            state_out, statCycles,
            committed_inst + i * TOTAL_NUM_CPUS);
    }
}

