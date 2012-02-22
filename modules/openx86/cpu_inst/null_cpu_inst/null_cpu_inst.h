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
 * @brief Template Asim cpu pipeline instruction
 */

#ifndef _NULL_CPUINST_
#define _NULL_CPUINST_

#include "asim/syntax.h"

#include "asim/provides/isa.h"

#include "asim/mm.h"
typedef class mmptr<class CPU_INST_CLASS> CPU_INST;

class LAST_INST_CLASS
{
 private:
    CPU_INST lastInstruction;
    
 public:
    LAST_INST_CLASS()
    {
        lastInstruction = NULL;
    }
    ~LAST_INST_CLASS();
    
    void SetLastInst(CPU_INST li) { lastInstruction = li; }
    CPU_INST GetLastInst(void) { return lastInstruction; }
};

class CPU_INST_CLASS : public ASIM_MM_CLASS<CPU_INST_CLASS>,
    public ASIM_ITEM_CLASS
{
    
    static UID_GEN64    staticUidCtr;   ///< static ID to assign uniqueId
    const UINT64        uid;            ///< unique ID
    ASIM_INST           aInst;       	//the pointer to the architectural instruction
    HW_CONTEXT          hwc; 
    UINT32              hwcNum;
    UINT32              cpuNum;        // the cpu number this instruction belongs to
    bool                Committed;      
    CPU_INST next;      //< next (in time, i.e. younger) instruction
    CPU_INST prev;      //< prev (in time, i.e. older) instruction
    static LAST_INST_CLASS lastInst[128][MAX_TOTAL_NUM_HWCS];  // up to 128 cpu's

    public:
    CPU_INST_CLASS(const UINT32 cpunum, HW_CONTEXT hwc, const UINT32 hwcNum, ASIM_INST ainst); 
    ASIM_INST GetArchInst() const { return aInst; }
    HW_CONTEXT GetHWC() const { return hwc; }
    UINT32 GetHWCNum() const { return hwcNum; }
    UINT32 GetCpu() const { return cpuNum; }

    void DumpTrace() {}
    void DoRead() {}
    void DoWrite() {}
    bool IsMemRead() const { return aInst->IsMemRead(); }
    bool IsMemWrite() const { return aInst->IsMemWrite(); }
    bool IsMemOp() const { return IsMemRead() || IsMemWrite(); }
    void Issue(UINT64 cycle) {hwc->Issue(cycle, GetArchInst());};
    void Execute(UINT64 cycle);
    bool IsNonSequentialPC() const { return aInst->IsNonSequentialPC(); }
    void Commit(UINT64 cycle);
    void Unlink();
    // link this instruction into the linked list of instructions
    void Link();
    void NullAllSrcDep() {};
    void NullAllDstDep() {};

    // Pass the ASIM_INST functions along to the ASIM_INST data member
    // Originally this class was derived from ASIM_INST_CLASS, but that created memory
    //  issues with mmptr
    inline bool IsControlOp()const { return aInst->IsControlOp(); }
    inline IADDR_CLASS GetActualTarget() { return aInst->GetActualTarget(); }
    IADDR_CLASS GetNextVirtualPC() const { return aInst->GetNextVirtualPC(); }
    IADDR_CLASS GetVirtualPC() const { return aInst->GetVirtualPC(); }
};

#endif //_NULL_CPUINST_
