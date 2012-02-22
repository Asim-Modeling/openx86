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
 * *                                                              *
 * *****************************************************************
 */
 
/**
 * @file x86_micro-inst.cpp
 * @author Mohit Gambhir
 *
 */

#include <map>
#include <string>

// ASIM core
#include "asim/trace.h"

// ASIM public modules
#include "asim/provides/isa.h"
#include "asim/provides/macro_inst.h"

UINT32 MAX_MICRO_INST = REC_INST_STAT ? 65000 : 5000;

ASIM_MM_DEFINE(MICRO_INST_CLASS, MAX_MICRO_INST);
UID_GEN64 MICRO_INST_CLASS::staticUidCtr = 1;  // make sure this is 1 --slechta
UINT32 MICRO_INST_CLASS::element_count=0;

// ----------------------------------------------------------------------------
MICRO_INST_CLASS::MICRO_INST_CLASS( SW_CONTEXT sw_context, UINT64 _uid) // Constructor
:   ASIM_MM_CLASS<MICRO_INST_CLASS>(_uid? _uid: (staticUidCtr += (1<<LOG2_X86MICRO_UID_BUMP),
                                    (UINT64)staticUidCtr)),
    uid(ASIM_MM_CLASS<MICRO_INST_CLASS>::GetMMUid()),
    swc(sw_context),
    asimSchedulerContextSwitch(false)
{    
    vpc.SetMacro(0,0);
}

MICRO_INST_CLASS:: ~MICRO_INST_CLASS()
{
    macroInst = NULL;
}

// ----------------------------------------------------------------------------
bool
MICRO_INST_CLASS::IsAnySrcAnImplicitDst()
{
    // Return true for uops that have one or more 
    // src regs that are also implicit destinations
    return false;
}

// ----------------------------------------------------------------------------
void
MICRO_INST_CLASS::SetDis(
    const char* arg_disasm)
{
    strncpy(disassembly,arg_disasm,255);
    disassembly[255] = 0;
}

void
MICRO_INST_CLASS::CreateNewMacroOp(
    UINT64 LIP)
{
    macroInst = new MACRO_INST_CLASS(swc);
}

