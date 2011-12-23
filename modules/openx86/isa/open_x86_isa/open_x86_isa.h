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
 * @file open_x86_isa.h
 * @author Mohit Gambhir
 *
 */

#ifndef _OPEN_X86_ISA_
#define _OPEN X86_ISA_

#include "asim/mm.h"

typedef class mmptr<class MICRO_INST_CLASS> ASIM_INST;
typedef class MICRO_INST_CLASS ASIM_INST_CLASS;

typedef class mmptr<class MACRO_INST_CLASS> ASIM_MACRO_INST;
typedef class MACRO_INST_CLASS ASIM_MACRO_INST_CLASS;

// ASIM public modules
#include "asim/provides/register_names.h"
#include "asim/provides/micro_inst.h"
#endif
