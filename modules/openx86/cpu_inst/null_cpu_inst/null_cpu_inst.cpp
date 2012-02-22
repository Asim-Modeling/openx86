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
 * @brief Template Asim cpu pipeline instruction implementation
 */

#include "asim/syntax.h"
#include "asim/provides/cpu_inst.h"

#define MAX_CPU_INST 2

ASIM_MM_DEFINE(CPU_INST_CLASS, MAX_CPU_INST);
UID_GEN64 CPU_INST_CLASS::staticUidCtr = 0;

/************************************************
 * CLASS LAST_INST_CLASS
 ************************************************/
LAST_INST_CLASS CPU_INST_CLASS::lastInst[128][MAX_TOTAL_NUM_HWCS];

/// LAST_INST_CLASS destructor
LAST_INST_CLASS::~LAST_INST_CLASS()
{
}

/************************************************
 * CLASS CPU_INST_CLASS
 ************************************************/

/*****************************************************************
 *
 *Instruction Creation stuff
 *
 ****************************************************************/
CPU_INST_CLASS::CPU_INST_CLASS(const UINT32 cpu, HW_CONTEXT hwc, const UINT32 hwc_num, ASIM_INST archinst)
    : ASIM_MM_CLASS<CPU_INST_CLASS>(staticUidCtr++),
      uid(ASIM_MM_CLASS<CPU_INST_CLASS>::GetMMUid()),
      hwc(hwc),
      hwcNum(hwc_num),
      cpuNum(cpu), 
      Committed(false),
      next(NULL),
      prev(NULL)
{   
    aInst = archinst;
}

void
CPU_INST_CLASS::Execute(UINT64 cycle)
{ 
    hwc->Execute(GetArchInst());
}

///
// Link 'this' to the previously linked instruction (end of the list).
// Instructions are linked so that we can quickly determine age order when
// killing, etc.
//
void
CPU_INST_CLASS::Link()
{
    UINT32 hwcNum = this->GetHWCNum();
    UINT32 cpuNum = this->GetCpu();

    CPU_INST li = lastInst[cpuNum][hwcNum].GetLastInst();

    if (li != NULL)
    {
        li->next = this;
        prev = li;
        next = NULL;
    }

    lastInst[cpuNum][hwcNum].SetLastInst(this);
}

/*
 * Unlink 'this' from the previously linked instruction.  Instructions are
 * linked so that we can quickly determine age order when killing,
 * etc.
 */
void
CPU_INST_CLASS::Unlink()
{  
    UINT32 hwcNum = this->GetHWCNum();
    UINT32 cpuNum = this->GetCpu();    

    if (lastInst[cpuNum][hwcNum].GetLastInst() == this)
    {
        lastInst[cpuNum][hwcNum].SetLastInst(prev);
    }   
    if (next)
    {
        next->prev = prev;
    }   
    if (prev)
    {
        prev->next = next;
    }   
    next = NULL;
    prev = NULL;
    
    NullAllSrcDep();
    NullAllDstDep();    
}

void
CPU_INST_CLASS::Commit(UINT64 cycle) 
{ 
    hwc->Commit(cycle, GetArchInst(), 1);
    hwc->DoWrite(GetArchInst());
    Committed = true; 
    Unlink();
}


