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
 * @brief Simple chip class
 */


#ifndef _CHIP_CLASS_
#define _CHIP_CLASS_

// ASIM core
#include "asim/syntax.h"
#include "asim/module.h"
#include "asim/port.h"

// ASIM messages


// ASIM sub-modules
//    either use full include or just typedef

//#include "asim/provides/cpu.h"
typedef class ASIM_CPU_CLASS * ASIM_CPU;


// ASIM sub-algorithms
//    either use full include or just typedef



/**
 * Class ASIM_CHIP_CLASS
 *
 *
 */

typedef class ASIM_CHIP_CLASS * ASIM_CHIP;

class ASIM_CHIP_CLASS : public ASIM_MODULE_CLASS
{
  private:

    // Submodules
    ASIM_CPU  * cpu;
    UINT32 chipNum;

    // Algorithms

    // Ports


    // Statistics



    // Other CHIP private variables


    // CHIP private methods


  public:
    // CHIP public variables


    // Constructor
    ASIM_CHIP_CLASS(ASIM_MODULE parent, const char * const name, UINT32 chip_num);

    // Destructor
    ~ASIM_CHIP_CLASS();

    // Required by ASIM
    bool InitModule();

    // Do a cycle of work...
    void Clock (UINT64 cycle);

    // Additional CHIP public methods
    void DumpStats(STATE_OUT state_out, UINT64 statCycles, UINT64 * committed_insn);


};

#endif /* __CHIP_CLASS_ */
