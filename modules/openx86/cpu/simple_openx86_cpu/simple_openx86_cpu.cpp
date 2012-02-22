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

// generic
#include <iostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/module.h"
#include "asim/trace.h"
#include "asim/cmd.h"

// ASIM public modules
#include "asim/provides/cpu.h"
#include "asim/provides/chip.h"

/**
 * @brief Instantiate simple
 */
ASIM_CPU_CLASS::ASIM_CPU_CLASS (
    ASIM_MODULE parent,        ///< parent module
    const char *name,          ///< name of this module
    const UINT32 cpu_num,      ///< cpu number of this module
    const UINT32 chip_num,     ///< chip number of this module
    const UINT32 hwc_priority)
    : ASIM_MODULE_CLASS(parent, name),
      hwc(ASIM_REGISTRY(this), chip_num * TOTAL_NUM_CPUS + cpu_num, 0, hwc_priority),
      chipNum(chip_num),    
      cpuNum(cpu_num),
      inst(NULL)
{
    ASIM_SYSTEM sys = GetParentOfType<ASIM_SYSTEM>();
    
    ASSERTX(NUM_HWCS_PER_CPU <= 1);
   
    WARMUP_MANAGER warmup = sys->GetWarmupManager();
    ASSERTX(warmup != NULL);
    warmup->RegisterHWC(&hwc);

    CONTEXT_SCHEDULER context_scheduler = GetParentOfType<ASIM_SYSTEM>()->GetContextScheduler();
    ASSERTX(context_scheduler != NULL);    
    context_scheduler->AddHardwareContext(&hwc);
    
    nextPc.Set(0);
    // Initialize stats
    memset(instsRetired, 0, sizeof(instsRetired));

    // Register for stats
    ASIM_MODULE cpu = this;
    while (cpu && string(cpu->Name()).find("CPU") != 0) 
    {
      cpu = cpu->GetParent();
    }
    cpu->RegisterState(instsRetired, NUM_HWCS_PER_CPU, 
                        "RetiredInstructions", "Retired Instructions");

    // HACK! When using clockserver system, we need to tell it the module associated to
    // a given cpu number, in order to obtain the correct cycle for each cpu.
    sys->SYS_SetCpu2Module(chipNum * TOTAL_NUM_CPUS + cpuNum, this);

    //this core was written to be able to increment a macro counter, which will just 
    //increment on every isntruction for ipf
    sys->SYS_CanHaveMicroOps(true);

}

/**
 * @brief Free memory we have allocated
 */
ASIM_CPU_CLASS::~ASIM_CPU_CLASS ()
{
}

/**
 * @brief Initialize this module and submodules
 */
bool  ///< @return true on success
ASIM_CPU_CLASS::InitModule (void) 
{
    bool ok = true;

    TRACE(Trace_Sys, cout << Name() << ": InitModule: Initializing submodules..." << endl);

    return ok;
}


/**
 * @brief Fetch the next instruction
 */
CPU_INST
ASIM_CPU_CLASS::FetchNextInstruction(
    const UINT64 cycle)        ///< current cycle
{
    ASIM_INST ainst;
 
    ainst = hwc.Fetch(cycle, nextPc);
        
    // if the feeder has nothing to give, stop fetching, and don't do
    // anything else.
    if (ainst == NULL)
    {
        return NULL;
    }
    else
    {
        if (ainst->HadFault())
        {
            hwc.GetSWC()->GetIFeeder()->Kill(ainst,false,true);

            //nextPc= ainst->GetFaultPC();
            //ainst = hwc.Fetch(cycle,nextPc);
        }
   } 
    CPU_INST cInst;
    
    cInst = new CPU_INST_CLASS(cpuNum, &hwc, hwc.GetUID(), ainst);
    
    TRACE(Trace_Sys, cout << "\tFetch: ");
    TRACE(Trace_Sys, cInst->DumpTrace());
                
    return cInst;
}

/**
 * A loop iterates and fetches, executes, and commits instructions
 */
void
ASIM_CPU_CLASS::FetchExecuteCommitInstructions(
    const UINT64 cycle)
{
    UINT32 numMemOpsHandled = 0;
    UINT32 numOpsHandled = 0;
    bool executedMemInst = true;

    double ipc = (double) instsRetired[0]/(cycle+1);

    // bandwidth of 3 instructions per cycle
    while (numOpsHandled < ISSUE_BW && ipc < IPC_TARGET)
    {
        // if inst != NULL, that means that an instruction from the
        // previous cycle stalled and needs to be re-executed, so
        // don't fetch a new instruction
        if (inst == NULL)
        {
            inst = FetchNextInstruction(cycle);
            
            // no SWC for this HWC
            if (inst == NULL)
            {
                break;
            }
        }

        numOpsHandled++;        

        // we only do 1 mem op per cycle, so if we get more than 1 in
        // 1 cycle, break out and do it the next cycle
        if (inst->IsMemOp() && (numMemOpsHandled > 0))
        {
            break;
        }

        // if inst is a MemOp, pass it to the mbox
        //        if (inst->IsMemOp()) this failed with instruction srlz.i
        if (inst->IsMemRead() || inst->IsMemWrite())
        {
            // Need to go ahead and issue and execute for the x86 feeder, 
            //   so we'll know the proper address in the Mbox
            inst->Issue(cycle);
            inst->Execute(cycle);

            // mbox returns flag indicating if this inst needs to be replayed
            //executedMemInst = mbox.ExecuteMemOp(inst, cycle);
            numMemOpsHandled++;
        }

        // if it's not a memop or it is and it didn't replay, commit
        // the instruction
        if (!inst->IsMemOp() || (inst->IsMemOp() && executedMemInst))
        {
            if (!inst->IsMemOp())
            {
                inst->Issue(cycle);
                inst->Execute(cycle); 
            }
            TRACE(Trace_Sys, cout << "\tCommit: ");
            TRACE(Trace_Sys, inst->DumpTrace());



            // before we destroy this instruction, let's calculate the
            // next PC/IP
            if (inst->IsNonSequentialPC()) 
            {
                if (inst->IsControlOp())
                {
                   nextPc = inst->GetActualTarget();
                }
                else
                {
                   nextPc = inst->GetNextVirtualPC();
                }
            } 
            else if(inst->GetArchInst()->HadFault())
            {
                nextPc = inst->GetNextVirtualPC();

            }
            else 
            {
                if (inst->IsControlOp()) 
                {
//                    nextPc = inst->GetCtrlInfo()->GetActualTarget();
                      nextPc = inst->GetActualTarget();
                } 
                else 
                {
                    nextPc = inst->GetVirtualPC().Next();
                }
            }


            inst->Commit(cycle);
            inst->DoWrite();

            if (hwc.GetSWC())
            {
                IFEEDER_BASE feeder = hwc.GetSWC()->GetIFeeder();
                FEEDER_SYSTEM_EVENT_CLASS system_event_output;
                for (int event = NULL_FEEDER_SYSTEM_EVENT + 1;
                     event < SYSTEM_EVENT_LAST;
                     event++)
                {
                    FEEDER_SYSTEM_EVENT_TYPES event_enum = FEEDER_SYSTEM_EVENT_TYPES(event);
                    while (feeder->IsSystemEventTypePending(inst->GetArchInst(), event_enum))
                    {
                        TRACE(Trace_Sys, cout<< "Handle system event " << event_enum << endl);
                        system_event_output=feeder->HandleSystemEvent(inst->GetArchInst(), event_enum);
                        if(event_enum ==IRQ_FEEDER_SYSTEM_EVENT)
                        {
                            nextPc =system_event_output.GetIrqPC();
                        }
                    }
                    
                }
            }

            asimSystem->SYS_CommittedInsts(chipNum * TOTAL_NUM_CPUS + cpuNum)++;
            

            //This is always true for Ipf, but only at the end of a macro inst is it true in x86
            if(inst->GetArchInst()->IsEndOfMacro())
            {
               asimSystem->SYS_CommittedMacroInsts(chipNum * TOTAL_NUM_CPUS + cpuNum)++;

            }
            instsRetired[0]++;
            
            // no further need for this inst, so destroy it.
            inst->Unlink();
            inst->NullAllDstDep();
            inst->NullAllSrcDep();
            
            // Must null out the instruction so that we know the next
            // time we try to fetch that it's ok to do so.
            inst = NULL;
        }
    }
}

/**
 * @brief Perform one cycle of work.
 */
void
ASIM_CPU_CLASS::Clock (
    const UINT64 cycle)  ///< current cycle
{    
    TRACE(Trace_Sys, cout << cycle << ": " << Name() << cpuNum << endl); 

    // Man, this is &*!% ugly.  It *should* start at PC=0 and return a branch
    // who's target is the correct starting location.  I really hope this is
    // fixed.... Eric
    if (cycle == 0 && hwc.GetSWC())
    {
        UINT64 startPC = hwc.GetSWC()->GetIFeeder()->GetFirstPC(hwc.GetSWC()->GetFeederStreamHandle());
        nextPc = IADDR_CLASS(startPC);
    }
    
    FetchExecuteCommitInstructions(cycle);

    // clocking the mbox allows the mbox to do things that aren't
    // related to a new incoming instruction.
    //mbox.Clock(cycle);
}
