// -*- C++ -*-
/*
 * *****************************************************************
 * *                                                               *
 * *   Copyright (c) Intel Corporation, 2003                       *
 * *                                                              *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Intel     *
 * *   Corporation.  Possession, use,  duplication  or             *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Intel Corporation. *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
 
/**
 * @file open_x86_micro_inst.h
 * @author Mohit Gambhir
 *
 */



#ifndef _OPEN_X86_MICRO_INST_
#define _OPEN_X86_MICRO_INST_


#include "asim/mm.h"

typedef class mmptr<class MICRO_INST_CLASS> MICRO_INST;
typedef class mmptr<class MACRO_INST_CLASS> MACRO_INST;

#include <string>
#include <list>


// ASIM core
#include "asim/mesg.h"
#include "asim/syntax.h"
#include "asim/trace.h"
#include "asim/event.h"
#include "asim/arch_register.h"
#include "asim/memory_reference.h"
#include "asim/item.h"

// ASIM public modules
#include "asim/provides/iaddr.h"

typedef class SW_CONTEXT_CLASS* SW_CONTEXT;
#define MAX_PARTS 16
#define MAX_EXECCLASS 32

// Begin Uop struct defs
// Basic uop struct and definitions.Can be extended and/or modified
// Taken largely from Brian Slechta's Replay Transmogrifier
// http://www.crhc.illinois.edu/ACS/tools/rpt/about_rpt.html
typedef UINT32 Uop_Opcode; 
#define UOP_NSRC 10
typedef enum Uop_Dst_Regs_Enum {
  UOP_DST_A       = 0,
  UOP_DST_FLAGS   = 1,
  UOP_NDSTS       = 2,
} Uop_Dst_Regs;

struct Uop {
    Uop_Opcode opcode;           // self explanatory
    UINT32 cond;                 // conditional op direction
    UINT32 properties;           // used to classify instructions.
    UINT64 srcs[UOP_NSRC];       // Src register values
    UINT64 dsts[UOP_NDSTS];      // Destination register values
    UINT32 immtype;              // First immediate value type
    UINT64 immvalue;             // First immediate value
    UINT32 immtype2;             // Second immediate value type
    UINT64 immvalue2;            // Second immediate value
    UINT64 seg_reg;              // segment register override for mem ops
    UINT8  scale;                // load and lea ops use this for addressing
    UINT64 mem_mask;             // size of data loaded or stored in mem op
    bool   is_start_uop;         // first uop decoded from macro-op
    bool   is_end_uop;           // last uop decocded from macro-op
    INT8   fp_stack_pre_delta;   // stack top manipulation before uop execution
    INT8   fp_stack_mid_delta;   // stack top manipulation between src regread and dst regread
    INT8   fp_stack_post_delta;  // stack top manipulation after dst regread
    UINT8  macro_size;           // Size of original macro operation 
};

// End Uop struct defs

class MICRO_INST_CLASS : public ASIM_MM_CLASS<MICRO_INST_CLASS>,
    public ASIM_ITEM_CLASS
{
    static UID_GEN64 staticUidCtr;   ///< static ID to assign uniqueId
    const UINT64 uid;       ///< unique ID
    static UINT32 element_count;

    SW_CONTEXT          swc;            // software context this instruction is from
    IADDR_CLASS         vpc;            // Virtual PC
    IADDR_CLASS         nextVPC;        // Virtual PC of the next syllable on nextVPC or 
                                        // the controlPC (unless is nonconsecutive)
    IADDR_CLASS         actualTarget;   //this should be equal to either the
    MACRO_INST          macroInst;
    Uop uop;

    bool asimSchedulerContextSwitch;
    bool affectsNextMacroFetch;
    bool affectsNextMacroDecode;
    bool affectsNextMicroDecode;
    char disassembly[256];
    bool branchChangesMachineState;


  public:
    MICRO_INST_CLASS(SW_CONTEXT sw_context = NULL, UINT64 desired_uid=0); //CONS
    ~MICRO_INST_CLASS();
    // Set functions
    void SetAsimSchedulerContextSwitch(const bool cs);
    void SetVirtualPC(IADDR_CLASS const new_vpc);
    void SetNextVirtualPC(IADDR_CLASS const next_vpc);
    void SetAffectsNextMicroDecode() {affectsNextMicroDecode= true;}
    void SetAffectsNextMacroDecode() {affectsNextMacroDecode= true;}
    void SetAffectsNextMacroFetch() {affectsNextMacroFetch= true; }
    void SetDis(const char*  arg_disasm);
    void SetBranchChangesMachineState(bool force) { branchChangesMachineState = force; }


    // Get functions
    SW_CONTEXT GetSWC() { return swc;}
    bool GetAsimSchedulerContextSwitch() const;
    IADDR_CLASS GetVirtualPC() const;
    MACRO_INST GetMacroInst() { return macroInst; }
    inline IADDR_CLASS GetActualTarget();
    UINT64 GetUid() const;
    IADDR_CLASS GetNextVirtualPC() const;
    bool GetBranchChangesMachineState() const { return branchChangesMachineState; }
    inline const char* GetDisassembly() const { return &disassembly[0]; }

    // Uop related info
 	inline bool IsEscape() const;
    bool IsAnySrcAnImplicitDst();
    bool AffectsNextMicroDecode() {return affectsNextMicroDecode;}
    bool AffectsNextMacroDecode() {return affectsNextMacroDecode;}
    bool AffectsNextMacroFetch() {return affectsNextMacroFetch; }
    // UOP state
    //
    // A UOP reports that it is the end of a macro instruction when it
    // might be the end, not when it actually is.  If the last instruction
    // in a UOP translation is a jump back to the UOP flow then it may not
    // be the true end of macro.  The dynamic test for end returns true iff
    // the UOP flow is really complete.
    //
    inline bool IsDynamicEndOfMacro() const;
    inline bool IsStaticEndOfMacro() const { return false; }
    bool IsStartOfMacroFlow(void) const { return true; };
    inline bool HadFault() const { return false; }
    inline bool GetUjumpTaken() const {return false;}
    inline UINT32 GetFaultUipTarget() {return 0;}
    inline bool IsControlOp()const {return false;}
    inline bool IsMemRead() const {return false;}
    inline bool IsMemWrite() const {return false;}
    inline bool IsNonSequentialPC() const {return false;}
    inline bool IsEndOfMacro() const {return false;}
    
    void CreateNewMacroOp(UINT64 LIP);



}; // class MICRO_INST_CLASS 

/***********************************************************************************
 *
 * Include of the software context moved until after the class definition so
 * that the software context can have inlined functions that use asim inst functions
 *
 *************************************************************************************/
#include "asim/provides/macro_inst.h"
#include "asim/provides/software_context.h"

std::ostream& operator<<(std::ostream& o, const MICRO_INST inst);

/// Set the syllable's VPC (virtual program counter)

inline void
MICRO_INST_CLASS::SetAsimSchedulerContextSwitch(
    const bool cs)
{
    asimSchedulerContextSwitch = cs;
}

inline void 
MICRO_INST_CLASS::SetVirtualPC(
    IADDR_CLASS const new_vpc)
{ 
    vpc = new_vpc; 
}

/// Record the PC of the next correct-path syllable
inline void 
MICRO_INST_CLASS::SetNextVirtualPC(
    IADDR_CLASS const next_vpc) 
{ 
    nextVPC = next_vpc; 
}


inline bool
MICRO_INST_CLASS::GetAsimSchedulerContextSwitch() const
{
    return asimSchedulerContextSwitch;
}

inline IADDR_CLASS 
MICRO_INST_CLASS::GetVirtualPC() const 
{
    return vpc; 
}

inline IADDR_CLASS
MICRO_INST_CLASS::GetActualTarget() 
{
    return actualTarget;
}

inline UINT64 
MICRO_INST_CLASS::GetUid() const
{
    return uid;
}

inline IADDR_CLASS
MICRO_INST_CLASS::GetNextVirtualPC() const
{ 
    return nextVPC; 
}

inline bool
MICRO_INST_CLASS::IsEscape() const
{
    // check the opcode and return true if it is Escape uop;
    return false;
}             

inline bool
MICRO_INST_CLASS::IsDynamicEndOfMacro(void) const
{
    return IsStaticEndOfMacro() && ! GetUjumpTaken();
}

#endif
