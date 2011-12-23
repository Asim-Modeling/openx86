// -*- C++ -*-
/*
 * *****************************************************************
 * *                                                               *
 * *   Copyright (c) Intel Corporation, 2003                       *
 * *                                                               *
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
 * @file x86_macro_inst.h
 * @author Chris Weaver, Mark Charney, Mohit Gambhir
 *
 */

#include "asim/mm.h"
typedef class mmptr<class MACRO_INST_CLASS> MACRO_INST;


#ifndef _OPEN_X86_MACRO_INST_
#define _OPEN_X86_MACRO_INST_

#include <string>
#include <list>


// ASIM core
#include "asim/mesg.h"
#include "asim/syntax.h"
#include "asim/trace.h"
#include "asim/event.h"
#include "asim/arch_register.h"
#include "asim/memory_reference.h"

#if MAX_PTHREADS > 1
#include "asim/smp.h"
#endif

// ASIM public modules
#include "asim/provides/iaddr.h"
#include "asim/provides/cpu_mode.h"
#include "asim/provides/isa.h"

extern "C" {
#include "xed-interface.h"
}


//using namespace XED;


typedef class SW_CONTEXT_CLASS* SW_CONTEXT;

// PM specific files


////////////////////
// Stub definitions
//

// 
// Global defines and constants
//

class MACRO_INST_CLASS : public ASIM_MM_CLASS<MACRO_INST_CLASS>,
    public ASIM_ITEM_CLASS
{
    // X86 CISC Macro-instruction
  public:
    enum
    {
        MAX_BYTES_PER_INST = 16,
        MAX_LOAD_REFS = 4,
        MAX_STORE_REFS = 1
    } consts;

  private:
    static UID_GEN64 staticUidCtr;   ///< static ID to assign uniqueId
    const UINT64 uid;       ///< unique ID
    char disassembly[256];
#if MAX_PTHREADS > 1
    static pthread_mutex_t xed_mutex;   // a lock for XED thread safety
# define XED_ENTER   pthread_mutex_lock  ( & xed_mutex );
# define XED_LEAVE   pthread_mutex_unlock( & xed_mutex );
#else
# define XED_ENTER
# define XED_LEAVE
#endif

    //
    // Memory reference descriptors.  Virtual references may cross any
    // number of pages.  Consequently, the instruction does not hold
    // the physical translation.  Call the feeder's DTranslate for
    // physical addresses.
    //
    MEMORY_VIRTUAL_REFERENCE_CLASS loadRefs[MAX_LOAD_REFS];
    MEMORY_VIRTUAL_REFERENCE_CLASS storeRefs[MAX_STORE_REFS];
    UINT32 nLoadRefs;
    UINT32 nStoreRefs;

    bool isLoad;
    bool isStore;
    UINT64 VirtualAddressMemFeeder;

    PTR_SIZED_UINT      traceID;        ///< Trace identifier

    UINT8               instBytes[MAX_BYTES_PER_INST]; ///< raw x86 instruction 
    UINT8               *opcodeBytes;   // PGS same as instbytes, but no prefixes
    bool                hasPrefix;      // PGS whether this inst has a prefix
    
    SW_CONTEXT          swc;            ///< software context this
                                        ///< instruction is from
    
    IADDR_CLASS         vpc;            ///< Virtual PC
    IADDR_CLASS         actualTarget;   ///< target of taken branch

    CPU_MODE_TYPE       cpuMode;        // Machine mode
    bool                forceCpuMode;   // Force machine into this mode

    bool feederContextSwitch;
    bool asimSchedulerContextSwitch;
    bool isRepeatOp;
    bool isControlOp;
    bool isPauseOp;                     // PAUSE instruction
    bool isIdlePauseOp;                 // PAUSE instruction in kernel idle loop
    bool isKernelInstr;
    bool macroDefinesTarget;
    UINT32 oSize;  //I got no idea what this is, but snowy needs it to do some prefix
                   //analysis on the macro op. It come from doing an unalias on the 
                   //UOP_ALIAS(*aliases, Uop_Alias_OSize);

    UINT32 bytesToFetch[2];    

    UINT32 patternEntry;
    UINT64 ipDelta; 

    // The following two variables are used to decide if there was a fetch fault
    bool fetchv2pFault[2];
    UINT32 fetchv2pFaultCode[2];
    bool decodeFault;
    UINT32 bytesOnPageOne;
    
    // Set by the macro feeder to avoid micro instructions execution (@ feeder2archlib module)
    bool overrideExecution;

    bool isSSCMarker;
    UINT32 SSCMarkerID;

    // xed decode cache
    xed_decode_cache_t cache;
    xed_decode_cache_entry_t* cacheEntries;


  public:
    UINT32 GetOsize() {return oSize;}
    void SetOsize(UINT32 arg) {oSize=arg;}

    void SetFetchFault(UINT32 page, bool arg) { fetchv2pFault[page]=arg;}
    bool GetFetchFault(UINT32 page) { return fetchv2pFault[page];}
    void SetFetchFaultCode(UINT32 page, UINT32 arg) { fetchv2pFaultCode[page]=arg;}
    int  GetFetchFaultCode(UINT32 page) { return fetchv2pFaultCode[page];}

    void SetPresentPageBytes(UINT32 arg) { bytesOnPageOne=arg; }
    UINT32 GetPresentPageBytes() { return bytesOnPageOne; }

    void SetInstructionSize(UINT32 arg) { vpc.SetLength(arg); }
    UINT32 GetInstructionSize() { return vpc.GetLength(); }

    //
    // Constructors, destructors, and initialization methods
    //
    // TO DO: why aren't the address, disassembly, and instruction bytes all
    // passed into the constructor?
    MACRO_INST_CLASS(SW_CONTEXT sw_context = NULL);
    ~MACRO_INST_CLASS() {delete cacheEntries;};

    /// Given the bundle bits and a syllable "address", initialize a syllable
    /// object with the info common to all syllable types
    void Init(const UINT8* const arg_inst_bytes,
              UINT64 const arg_vpc,
              int length);
    //
    // Modifiers
    //
    void SetPatternEntry(UINT32 arg_pattern_entry);

    void SetDis(const char* arg_disasm);
    void SetTraceID(PTR_SIZED_UINT tID);

    void SetVirtualPC(IADDR_CLASS const new_vpc);
    
    void SetLoadRef(UINT32 idx, const MEMORY_VIRTUAL_REFERENCE_CLASS& vRef);
    void SetStoreRef(UINT32 idx, const MEMORY_VIRTUAL_REFERENCE_CLASS& vRef);

    void SetAsimSchedulerContextSwitch(const bool cs);
    void SetFeederContextSwitch(const bool cs);

    void SetNonSequentialPc(const IADDR_CLASS next_vpc );

    void SetFetchBytes(UINT32 amount,UINT32 i);
    UINT32 GetFetchBytes(UINT32 i);

    UINT64 GetIpDelta();
    void SetIpDelta(UINT64 arg_ipdelta);

    // REP instructions
    void SetRepeatOp() { isRepeatOp = true; }
    bool IsRepeatOp() const { return isRepeatOp; }

    // PAUSE instructions
    void SetPauseOp() { isPauseOp = true; };
    bool IsPauseOp() const { return isPauseOp; };
    void SetIdlePauseOp() { isIdlePauseOp = true; };
    bool IsIdlePauseOp() const { return isIdlePauseOp; };

    // In the kernel?
    void SetKernelInstr() { isKernelInstr = true; };
    bool IsKernelInstr() const { return isKernelInstr; };

    // SSC marker instructions are no side effect instructions (essentially nops) 
    // and can be used as a run time tags to control simulation
    // Classic controller base module has command line argument parsing support to enable 
    // ssc markers.However the controller algorithm and x86 instruction feeder will have to be
    // extended to enable controller actions when ssc instructions are seen int he trace
    void SetSSCMarker(UINT32 id) { isSSCMarker = true; SSCMarkerID = id; };
    bool IsSSCMarker() const { return isSSCMarker; };
    UINT32 GetSSCMarkerID() const { assert(isSSCMarker); return SSCMarkerID; };

    //
    // Control Ops are a bit complicated for macro instructions.
    // Like most instruction definitions, a target address can be specified
    // (GetActualTarget) and a taken bit (GetActualTaken) are available.
    // The macro feeder also encodes whether actual target was set
    // (GetMacroDefinesTarget).  This bit is useful when a micro instruction
    // feeder is translating the macro instruction.  Some macro feeders
    // (e.g. LIT) do not determine the direction of a branch and would
    // leave the target undefined.  Some macro feeders (e.g. SoftSDV)
    // set the target and insist that the micro feeder honor the target.
    //
    void SetControlOp() { isControlOp = true; }
    bool IsControlOp() const { return isControlOp; }
    bool GetMacroDefinesTarget() const { return macroDefinesTarget; };
    IADDR_CLASS GetActualTarget() const;
    void SetActualTarget(IADDR_CLASS target);
    PUBLIC_OBJ_ASSERT(bool, ActualTaken, IsControlOp());         // Branch taken

    // *** IsCondControlOp() as implemented is WRONG ***
    //  SoftSDV doesn't indicate whether a branch is conditional and macro
    //  instructions aren't decoded in the macro feeder.  For now, assume
    //  that all branches are conditional.
  public:
    bool IsCondControlOp() const { return IsControlOp(); };

    // Branch is injected for an interrupt?  Force model to drain pipelines
    // before continuing.
    PUBLIC_OBJ_ASSERT(bool, BranchChangesMachineState, IsControlOp());
    
  public:
    // NLoadRefs --
    //   Number of separate virtual memory references made by the instruction.
    //   Most have at most 1.  A few (e.g. CMPS) have 2.
    UINT32 NLoadRefs() const;
    // Get the VA information for region 0 .. NLoadRefs()-1
    MEMORY_VIRTUAL_REFERENCE_CLASS GetLoadInfo(UINT32 n) const;

    // Equivalent store information...
    UINT32 NStoreRefs() const;
    MEMORY_VIRTUAL_REFERENCE_CLASS GetStoreInfo(UINT32 n) const;

    bool IsLoad() const {  return NLoadRefs() > 0 || isLoad ; }
    bool IsStore() const { return NStoreRefs() > 0 || isStore; }
   
    //used by mem-feeder
    void SetIsLoad() { isLoad = true; }
    void SetIsStore() { isStore = true; }
    void SetVa(UINT64 VirtualAddress){ VirtualAddressMemFeeder = VirtualAddress; }

    void SetDecodeFault(bool arg) { decodeFault=arg;}
    bool GetDecodeFault() {return decodeFault; }
    //
    // *** These memory info methods should go away as soon as all
    // *** models deal with the new multi-reference interface.
    //
    UINT64 GetLoadVA() const;
    UINT64 GetStoreVA() const;
    bool HasImm8() const;

    PUBLIC_OBJ(UINT64, PhysicalPC);                     // PA of the instruction

  private:
    bool goToExceptionHandler;

    /// Conditionally initialize the XED tables; Only takes effect once.
    void XEDConditionalInit(); 
    bool XEDGetState(xed_state_t& state);  
    xed_decoded_inst_t xed_decoded_inst;

  public:

    bool GetGoToExceptionHandler() {return goToExceptionHandler;}
    void SetGoToExceptionHandler(bool arg) {goToExceptionHandler=arg;}
    bool IsMemRead() const { return IsLoad() || IsPrefetch() || IsSemaphore(); }
    bool IsMemWrite() const { return IsStore() || IsSemaphore(); }

    // These flags not yet implemented...
    bool IsNonSequentialPC() const { return false; }
    bool IsSemaphore() const { return false; }
    bool IsPrefetch() const { return false; }


    const xed_decoded_inst_t* XEDDecode();
 

    const char* GetDisassembly();
    UINT64 GetUid() const;
    SW_CONTEXT GetSWC() { return swc;}
    PTR_SIZED_UINT GetTraceID() const;
    IADDR_CLASS GetVirtualPC() const;
    IADDR_CLASS GetNextVirtualPC() const;
    UINT32 GetLength() const;

    bool GetContextSwitch() const;
    bool GetAsimSchedulerContextSwitch() const;
    bool GetFeederContextSwitch() const;

    const UINT8* GetRawInstruction() const;
    const UINT8* GetRawOpcode();
    void SetRawInstruction(UINT8* const b);
    xed_decoded_inst_t* GetXedDecodedInst() { return &xed_decoded_inst; };

    UINT32 GetByteMask()
    {
        ASIMERROR("We need separate calls for loads and stores to support macro insts");
        return 0;
    }

    //
    // CPU mode describes the current execution mode.  Typically
    // the machine will be in either 32 bit protected or 64 bit mode.
    // ForceCpuMode() is a request from a macro feeder to a micro
    // feeder to set the machine state so it will interpret the
    // instruction in the requested mode.  Don't set this flag often,
    // since it requires modifying a number of registers.
    //
    inline void SetCpuMode(CPU_MODE_TYPE newMode, bool force = false);
    inline CPU_MODE_TYPE GetCpuMode(void) const { return cpuMode; };
    inline bool ForceCpuMode(void) const { return forceCpuMode; };
     
    string ToString(const string status = "");

    //------------------------------------------------------------------
    // IPF compatibility section --
    //     These functions are added for code that deals with both IPF
    //     and x86.

    // For now there are no injected macro instructions in the model.
    // IPF injects instructions for register stack engine traffic.
    bool IsInjectedInst() const { return false; }
    // IPF has some instructions that operate on cache tags
    bool IsTaccess() const { return false; }
    // Instructions are never predicated false on x86
    bool GetQP() const { return true; }
    // TypeL is an IPF long immediate encoding
    bool IsTypeL() const { return false; }


    //------------------------------------------------------------------
    // Micro instruction interface --
    //     Some models (e.g. Ape) can talk to either micro or macro instruction
    //     feeders.  The following are used only for micro instructions but
    //     must be present here to support a common interface.
    //

    void CreateNewMacroOp(UINT64 LIP) const {};
    MACRO_INST GetMacroInst(void) { return this; }
    bool HadFault(void) const { return false; }
    IADDR_CLASS GetFaultPC(void) const { return IADDR_CLASS(); }
    bool IsInterruptible(void) const { return true; }
    bool IsStaticEndOfMacro(void) const { return true; }
    bool IsDynamicEndOfMacro(void) const { return true; }
    bool IsStartOfMacroFlow(void) const { return true; };

    // Delete when gone from models...
    bool IsEndOfMacro(void) const { return true; }
    bool IsTrueEndOfMacro(void) const { return true; }

    //------------------------------------------------------------------

    bool GetOverrideExecution() const { return overrideExecution; }
    void SetOverrideExecution() { overrideExecution = true; }

}; // class MACRO_INST_CLASS 

std::ostream& operator<<(std::ostream& os, const MACRO_INST inst);

#include "asim/provides/software_context.h"

//////////////////////////////////////
// Inline methods for MACRO_INST_CLASS
//////////////////////////////////////

inline void 
MACRO_INST_CLASS::SetPatternEntry(
    UINT32 arg_pattern_entry) 
{ 
    patternEntry = arg_pattern_entry; 
}

/// Set the disassembly (string representation) of the syllable
inline void 
MACRO_INST_CLASS::SetDis(
    const char *arg_disasm)
{
    strncpy(disassembly,arg_disasm,255);
    disassembly[255] = 0;
}


/// Set the syllable's trace identifier 
inline void 
MACRO_INST_CLASS::SetTraceID(
    PTR_SIZED_UINT tID)
{ 
    traceID = tID; 
}

/// Set the syllable's bundle
inline void 
MACRO_INST_CLASS::SetRawInstruction(
    UINT8* const b)
{ 
    memcpy(&instBytes[0], b, MAX_BYTES_PER_INST);
    opcodeBytes = instBytes;    
    hasPrefix = false;
    
    // PGS check for prefixes and assign opcodebyte pointer appropriately
    //   note:  REX prefix check is done during GetOpcodeBytes() because the cpumode isn't known yet
    while ( opcodeBytes[0] == 0x67 ||   // address size
            opcodeBytes[0] == 0xF0 ||   // lock
            opcodeBytes[0] == 0xF2 ||   // rep
            opcodeBytes[0] == 0xF3 ||   // rep
            opcodeBytes[0] == 0x66 ||   // operand size
            opcodeBytes[0] == 0x2E ||   // CS override
            opcodeBytes[0] == 0x3E ||   // DS override
            opcodeBytes[0] == 0x26 ||   // ES override
            opcodeBytes[0] == 0x64 ||   // FS override
            opcodeBytes[0] == 0x65 ||   // GS override
            opcodeBytes[0] == 0x36 )    // SS override
    {
       opcodeBytes = &( opcodeBytes[1] ); // move opcode pointer to next byte
       hasPrefix = true;
    }
}


/// Set the syllable's VPC (virtual program counter)
inline void 
MACRO_INST_CLASS::SetVirtualPC(
    IADDR_CLASS const new_vpc)
{ 
    vpc = new_vpc; 
}


inline void
MACRO_INST_CLASS::SetAsimSchedulerContextSwitch(
    const bool cs)
{
    asimSchedulerContextSwitch = cs;
}

inline void
MACRO_INST_CLASS::SetFeederContextSwitch(
    const bool cs)
{
    feederContextSwitch = cs;
}

inline void 
MACRO_INST_CLASS::SetFetchBytes(
    UINT32 amount,
    UINT32 i)
{
    bytesToFetch[i] = amount;
}

inline UINT32 
MACRO_INST_CLASS::GetFetchBytes(
    UINT32 i)
{
    return bytesToFetch[i];
}


inline IADDR_CLASS
MACRO_INST_CLASS::GetActualTarget() const
{
    ASSERTX(GetMacroDefinesTarget());
    return actualTarget;
}

inline void
MACRO_INST_CLASS::SetActualTarget(IADDR_CLASS target)
{
    ASSERTX(IsControlOp());
    actualTarget = target;
    macroDefinesTarget = true;
}


inline const UINT8* 
MACRO_INST_CLASS::GetRawInstruction() const
{
    return instBytes;
}

// PGS
inline const UINT8* 
MACRO_INST_CLASS::GetRawOpcode() 
{
    // PGS: prefix detection in SetRawInst() can't do the REX prefix 
    //   check because the CPU mode isn't known yet, so we do it here
    while ((opcodeBytes[0] & 0xF0) == 0x40 && cpuMode==CPU_MODE_LONG_64)
    {
        opcodeBytes = &(opcodeBytes[1]);
        hasPrefix = true;
    }
    
    return opcodeBytes;
}

inline UINT64 
MACRO_INST_CLASS::GetIpDelta()  
{ 
    return ipDelta; 
}

inline void 
MACRO_INST_CLASS::SetIpDelta(
    UINT64 arg_ipdelta) 
{ 
    ipDelta = arg_ipdelta;
}


inline UINT64 
MACRO_INST_CLASS::GetUid() const
{
    return uid;
}


inline PTR_SIZED_UINT
MACRO_INST_CLASS::GetTraceID() const 
{
    return traceID; 
}

inline IADDR_CLASS 
MACRO_INST_CLASS::GetVirtualPC() const 
{
    return vpc; 
}

inline IADDR_CLASS 
MACRO_INST_CLASS::GetNextVirtualPC() const 
{
    return vpc.NextMacro(); 
}

inline UINT32 
MACRO_INST_CLASS::GetLength() const 
{
    return vpc.GetLength();
}

//Return true if there was a context switch either from the feeder or generated
//by the scheduler
inline bool
MACRO_INST_CLASS::GetContextSwitch() const
{
    return feederContextSwitch || asimSchedulerContextSwitch;
}

inline bool
MACRO_INST_CLASS::GetFeederContextSwitch() const
{
    return feederContextSwitch;
}
inline bool
MACRO_INST_CLASS::GetAsimSchedulerContextSwitch() const
{
    return asimSchedulerContextSwitch;
}
 
inline void
MACRO_INST_CLASS::SetCpuMode(CPU_MODE_TYPE newMode, bool force)
{
    cpuMode = newMode;
    forceCpuMode = force;
}


//
// NLoadRefs --
//    Number of individual memory regions read by the instruction.
//
inline UINT32
MACRO_INST_CLASS::NLoadRefs() const
{
    return nLoadRefs;
}

inline MEMORY_VIRTUAL_REFERENCE_CLASS
MACRO_INST_CLASS::GetLoadInfo(UINT32 n) const
{
    ASSERTX(n < nLoadRefs);
    return loadRefs[n];
}

inline void
MACRO_INST_CLASS::SetLoadRef(
    UINT32 idx,
    const MEMORY_VIRTUAL_REFERENCE_CLASS& vRef)
{
    ASSERTX(idx <= nLoadRefs);
    if (idx == nLoadRefs)
    {
        nLoadRefs += 1;
        ASSERTX(nLoadRefs <= MAX_LOAD_REFS);
    }

    loadRefs[idx] = vRef;
}


//
// NStoreRefs --
//    Number of individual memory regions written by the instruction.
//
inline UINT32
MACRO_INST_CLASS::NStoreRefs() const
{
    return nStoreRefs;
}

inline MEMORY_VIRTUAL_REFERENCE_CLASS
MACRO_INST_CLASS::GetStoreInfo(UINT32 n) const
{
    ASSERTX(n < nStoreRefs);
    return storeRefs[n];
}

inline void
MACRO_INST_CLASS::SetStoreRef(
    UINT32 idx,
    const MEMORY_VIRTUAL_REFERENCE_CLASS& vRef)
{
    ASSERTX(idx <= nStoreRefs);
    if (idx == nStoreRefs)
    {
        nStoreRefs += 1;
        ASSERTX(nStoreRefs <= MAX_STORE_REFS);
    }

    storeRefs[idx] = vRef;
}

inline UINT64
MACRO_INST_CLASS::GetLoadVA() const
{
    if(isLoad)
    {
        //this flow is only used the mem-feeder otherwise the macro instr class does not support this method
        return VirtualAddressMemFeeder;
    }
    
    ASIMERROR("GetLoadVA() not supported by x86 macro instr class");
    return 0;
}

inline UINT64
MACRO_INST_CLASS::GetStoreVA() const
{
    if(isStore)
    {   
        //this flow is only used the mem-feeder otherwise the macro instr class does not support this method
        return VirtualAddressMemFeeder;
    }

    ASIMERROR("GetStoreVA() not supported by x86 macro instr class");
    return 0;
}

inline bool
MACRO_INST_CLASS::HasImm8() const
{
    const UINT8 fob = instBytes[0];
 
    switch (fob) {
        // 2&3-byte opcodes
        //
        case 0x0F:
        {
                const UINT8 sob = instBytes[1];
                switch (sob) {
                    // 3-byte opcodes
                    //
                    case 0x3A: // all instructions in MAP3 have an imm8
                    // 2-byte opcodes
                    //
                    case 0x70: // PSHUFw/d/hw/lw
                    case 0x71: // Grp 12: PSrlw/raw/llw
                    case 0x72: // Grp 13: PSrld/rad/lld
                    case 0x73: // Grp 14: PSrlq/rdq/llq/ldq
                    case 0xA4: // SHLD
                    case 0xAC: // SHRD
                    case 0xBA: // BT
                    case 0xC2: // CMPps/pd/ss/sd
                    case 0xC4: // PINSRW
                    case 0xC5: // PEXTRW
                    case 0xC6: // SHUFps/pd
                        return true;
                    default:
                         break;
                }
        }
        break;
        
        // 1-byte opcodes
        //
        case 0x04: // ADD
        case 0x0C: // OR
        case 0x14: // ADC
        case 0x1C: // SBB
        case 0x24: // AND
        case 0x2C: // SUB
        case 0x34: // XOR
        case 0x3C: // CMP
        case 0x6A: // PUSH
        case 0x6B: // IMUL
        case 0x80: // Grp 1: ADD,OR,ADC,SBB,AND,SUB,XOR,CMP
        case 0x82: // Grp 1: ADD,OR,ADC,SBB,AND,SUB,XOR,CMP
        case 0x83: // Grp 1: ADD,OR,ADC,SBB,AND,SUB,XOR,CMP
        case 0xA8: // TEST
        case 0xB0: // MOV AL
        case 0xB1: // MOV CL
        case 0xB2: // MOV DL
        case 0xB3: // MOV BL
        case 0xB4: // MOV AH
        case 0xB5: // MOV CH
        case 0xB6: // MOV DH
        case 0xB7: // MOV BH
        case 0xC0: // Grp 2: ROL,ROR,RCL,RCR,SHL/SAL,SHR,SAR
        case 0xC1: // Grp 2: ROL,ROR,RCL,RCR,SHL/SAL,SHR,SAR
        case 0xC6: // Grp 11: MOV
        case 0xC8: // ENTER
        case 0xCD: // INT
        case 0xD4: // AAM
        case 0xD5: // AAD
        case 0xE4: // IN AL
        case 0xE5: // IN EAX
        case 0xE6: // OUT AL
        case 0xE7: // OUT EAX
            return true;
        case 0xF6: // Grp 3
            {
                const UINT8 modrm = instBytes[1];
                if ((modrm && 0x38) == 0) { // TEST
                    return true;
                }
            }
            return false;
        default:
            break;
    }

    return false;
}


#endif // _OPEN_X86_MACRO_INST_
