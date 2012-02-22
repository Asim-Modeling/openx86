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
 * @brief Simple chip class implementation
 */


// Generic C++
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <list>

// ASIM core
#include "asim/syntax.h"

// ASIM public module
#include "asim/provides/chip.h"

// ASIM sub-modules

#include "asim/provides/cpu.h"

// ASIM sub-algorithms

/**
 * Instantiate module and submodules.
 */
ASIM_CHIP_CLASS::ASIM_CHIP_CLASS (
    ASIM_MODULE parent, ///< parent module
    const char * const name,   ///< name of this module
    UINT32 chip_num)
    : ASIM_MODULE_CLASS(parent, name),
      chipNum(chip_num)
{

      // New each submodule

      NewClockDomain("CORE_CLOCK_DOMAIN", (float) 4);

      cpu = new ASIM_CPU [TOTAL_NUM_CPUS];
      
      for (UINT32 i=0; i<TOTAL_NUM_CPUS; i++) 
      {
          ostringstream name;
          name << "CPU_" << i;
          cpu[i] = new ASIM_CPU_CLASS(this, name.str().c_str(), i, chip_num);
      }

      // New each algorithm


      // Initialize the read ports


      // Initialize the write ports


      // Initialize the statistics



      // Initialize other CHIP variables

}

/**
 * Destroy module and submodules
 */
ASIM_CHIP_CLASS::~ASIM_CHIP_CLASS ()
{
    // Delete submodules
    for (UINT32 i=0; i<TOTAL_NUM_CPUS; i++)
    {
        delete cpu[i];
    }
    delete [] cpu;

}

/**
 * Initialize module and submodules
 */
bool
ASIM_CHIP_CLASS::InitModule ()
{
    bool ok = true;

    TRACE(Trace_Debug, cout << "0: Init Module " << Name() << endl);

    RegisterClock("CORE_CLOCK_DOMAIN", newCallback(this, &ASIM_CHIP_CLASS::Clock));

    // Submodule initilizations go here

    for(UINT32 i=0; i<TOTAL_NUM_CPUS; i++) 
    {
        ok &= cpu[i]->InitModule();
    }

    // Algorithms are not inited....


    // Additional initialization code...

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
ASIM_CHIP_CLASS::Clock (
    UINT64 cycle)        ///< current cycle
{
    // Read port input variables


    // Write port output variables


    TRACE(Trace_Sys, cout << cycle << ": Clocking - " << Name() << endl);

    // Read the input ports



    // Perform the module's activity


    // Write the output ports


    // Update statistics


    // Clock submodules

    for(UINT32 i=0; i<TOTAL_NUM_CPUS; i++) 
    {
        cpu[i]->Clock(cycle);
    }


}

void ASIM_CHIP_CLASS::DumpStats (STATE_OUT state_out, UINT64 statCycles, UINT64 * committed_inst)
{
    for (UINT32 i=0; i<TOTAL_NUM_CPUS; i++)
    {
        cpu[i]->DumpStats(state_out, statCycles,committed_inst + i * TOTAL_NUM_CPUS);
    }
}

