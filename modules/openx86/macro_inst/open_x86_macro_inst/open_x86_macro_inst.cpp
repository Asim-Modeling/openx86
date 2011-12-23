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
 * @file x86_macro_inst.cpp
 * @author Chris Weaver and Mark Charney 
 *
 */

#include <map>
#include <string>

// ASIM core
#include "asim/trace.h"

// ASIM public modules
#include "asim/provides/macro_inst.h"
#include "asim/provides/xed.h"


const UINT32 MAX_MACRO_INST = REC_INST_STAT ? 65000 : 5000*10;

ASIM_MM_DEFINE(MACRO_INST_CLASS, MAX_MACRO_INST);
UID_GEN64 MACRO_INST_CLASS::staticUidCtr = 0;

#if MAX_PTHREADS > 1
// a lock for XED thread safety
pthread_mutex_t MACRO_INST_CLASS::xed_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

MACRO_INST_CLASS::MACRO_INST_CLASS(
    SW_CONTEXT sw_context)
    : ASIM_MM_CLASS<MACRO_INST_CLASS>(staticUidCtr++),
      uid(ASIM_MM_CLASS<MACRO_INST_CLASS>::GetMMUid()),
      nLoadRefs(0),
      nStoreRefs(0),
      isLoad(0),
      isStore(0),
      traceID(0),
      swc(sw_context),
      cpuMode(CPU_MODE_UNKNOWN),
      forceCpuMode(false),
      feederContextSwitch(false),
      asimSchedulerContextSwitch(false),
      isRepeatOp(false),
      isControlOp(false),
      isPauseOp(false),
      isIdlePauseOp(false),
      isKernelInstr(false),
      macroDefinesTarget(false),
      overrideExecution(false),
      isSSCMarker(false)
{
    fetchv2pFault[0]=false;
    fetchv2pFault[1]=false;
    fetchv2pFaultCode[0]=0;
    fetchv2pFaultCode[1]=0;
    decodeFault=false;
    disassembly[0] = 0;
    memset(instBytes, 0, MAX_BYTES_PER_INST);
    goToExceptionHandler=false;
}

//
// Initialize a MACRO_INST 
//

void
MACRO_INST_CLASS::Init(
    const UINT8* arg_inst_bytes,
    UINT64 const arg_vpc,
    int length)
{
    memcpy(instBytes, arg_inst_bytes, MAX_BYTES_PER_INST);
    vpc.SetMacro(arg_vpc, length);
}


/// Conditionally initialize the XED tables; Only takes effect once.
void 
MACRO_INST_CLASS::XEDConditionalInit()
{
    static bool didInit = false;
    XED_ENTER
    if (! didInit)
    {
        UINT64 nentries = 16*1024;
        cacheEntries = (xed_decode_cache_entry_t*) malloc(nentries * 
                                           sizeof(xed_decode_cache_entry_t));
        xed_tables_init();
        xed_decode_cache_initialize(&cache, cacheEntries, nentries);
        didInit = true;
    }
    XED_LEAVE
}

bool
MACRO_INST_CLASS::XEDGetState(xed_state_t& state)
{

    xed_machine_mode_enum_t xmode = XED_MACHINE_MODE_INVALID;
    xed_address_width_enum_t xwidth = XED_ADDRESS_WIDTH_INVALID;

    //
    // This supports a limited set of the possible modes.  It assumes the
    // data and data sizes are the same.
    //
    switch (GetCpuMode())
    {
      case CPU_MODE_REAL:
      case CPU_MODE_V8086:
      case CPU_MODE_PROTECTED_16:
        xmode = XED_MACHINE_MODE_LEGACY_16;
        xwidth = XED_ADDRESS_WIDTH_16b;
        break;
      case CPU_MODE_PROTECTED_32:
        xmode = XED_MACHINE_MODE_LEGACY_32;
        xwidth = XED_ADDRESS_WIDTH_32b;
        break;
      case CPU_MODE_LONG_COMPATIBLE_16:
        xmode = XED_MACHINE_MODE_LONG_COMPAT_16;
        xwidth = XED_ADDRESS_WIDTH_16b;
        break;
      case CPU_MODE_LONG_COMPATIBLE_32:
        xmode = XED_MACHINE_MODE_LONG_COMPAT_32;
        xwidth = XED_ADDRESS_WIDTH_32b;
        break;
      case CPU_MODE_LONG_64:
        xmode = XED_MACHINE_MODE_LONG_64;
        xwidth = XED_ADDRESS_WIDTH_64b;
        break;
      case CPU_MODE_SMM:
      default:
        return false;
    }

    xed_state_set_machine_mode(&state, xmode);
    xed_state_set_address_width(& state, xwidth);
    xed_state_set_stack_address_width(&state, xwidth);
    return true;
}

const xed_decoded_inst_t*
MACRO_INST_CLASS::XEDDecode()
{
    xed_state_t machine_state;
    bool okay = XEDGetState(machine_state);
    if (okay)
    {
    
        // initialize before invoking decode cache
        XEDConditionalInit();

        XED_ENTER
        xed_decoded_inst_zero_set_mode(&xed_decoded_inst, &machine_state);

        xed_error_enum_t xed_error = xed_decode_cache(&xed_decoded_inst, 
                                                      GetRawInstruction(), 
                                                      GetLength(), &cache);
        XED_LEAVE
        if (xed_error == XED_ERROR_NONE)
        {
            return &xed_decoded_inst;
        }
    }
    return 0;
}

const char*
MACRO_INST_CLASS::GetDisassembly()
{ 
    if (disassembly[0] != 0)
    {
        return disassembly;
    }

    //
    // No disassembly available.  Use XED to compute one.
    //
    XEDConditionalInit();

    const xed_decoded_inst_t* decoded_inst = XEDDecode();
    if (decoded_inst)
    {
        char disasm[256];
	XED_ENTER
        xed_decoded_inst_dump(decoded_inst, disasm, 256);
	XED_LEAVE
        SetDis(disasm);
    }
    else
    {
        SetDis("<unknown macro inst>");
    }

    return disassembly;
}


/// Output operator for instruction
ostream &
operator << (
    ostream & os, 
    const MACRO_INST inst)
{
    if (inst)
    {
        // insert any other output statements here that you want
        os << inst->GetVirtualPC() 
           << " : " 
           << inst->GetDisassembly() 
           << " -- len="
           << inst->GetVirtualPC().GetLength()
           << ", m_uid="
           << inst->GetUid();
    }
    else
    {
        os << "NULL";
    }
    return os;
}
