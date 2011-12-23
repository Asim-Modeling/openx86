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
 * @file inst_addr.h
 * @author Chris Weaver and Mark Charney 
 *
 * @brief This file contains information about X86 instruction addresses.
 */

#ifndef _IADDR_
#define _IADDR_

// generic
#include <ostream>

// ASIM core
#include "asim/syntax.h"
#include "asim/mesg.h"
#include "asim/item.h"

typedef class IADDR_CLASS* IADDR;
/**
 * @brief X86 address 
 */
class IADDR_CLASS : public ASIM_SILENT_ITEM_CLASS
{
  private:
    //this is the macro address- lip
    UINT64 address;
    //this is the micro address- uip
    UINT32 microAddress;
    
    //these are bools to mark the start and end of a flow (a macro instruction)
    bool startOfFlow;
    bool endOfFlow;

    //the number of bytes that the instruction uses
    UINT32    length; /* bytes */

    // if true, the address requested with this is irrelevant because
    // we're starting up an istream on a HWC
    bool istreamStartup;

  public: 
    IADDR_CLASS(const UINT64 arg_addr,
                const UINT32 arg_micro_address,
                const UINT32 arg_length = 0,
                const bool arg_start_of_flow = 0,
                const bool arg_end_of_flow = 0); //CONS

    IADDR_CLASS(); //CONS

    IADDR_CLASS( const UINT64 arg_addr ); //CONS

    inline void SetLength(unsigned int arg_length);

    inline UINT64 Get(void) const;
    inline UINT32 GetLength(void) const;
    inline void SetMacro(const UINT64 arg_addr,
                          const unsigned int arg_length);
    inline void SetMacro(const UINT64 arg_addr);
     
    inline void SetMicro(const UINT32 arg_addr);
     
    inline void SetEOF(const bool arg_EOF);
    inline void SetSOF(const bool arg_SOF);
    inline void Set(UINT64 pc);
    inline void SetIstreamStartup();

    inline UINT64 GetMacro() const;
    inline UINT32 GetMicro() const;

    // All IADDR_CLASSes must provide GetUniqueAddr().  On x86 it is the
    // macro address.  On IPF it is the syllable level address.
    inline UINT64 GetUniqueAddr() const { return GetMacro(); };

    // All IADDR_CLASSes must provide GetAddr().  On x86 it is the
    // macro address.  On IPF it is the instruction level address.
    inline UINT64 GetAddr() const { return GetMacro(); };

    inline bool GetEOF();
    inline bool GetSOF();
    inline bool GetIstreamStartup() const;

    inline void Clear();
    IADDR_CLASS Next() const;
    IADDR_CLASS NextMacro() const;

};

inline
IADDR_CLASS::IADDR_CLASS(
    const UINT64 arg_addr,
    const UINT32 arg_micro_address,
    const UINT32 arg_length,
    const bool arg_start_of_flow,
    const bool arg_end_of_flow) //CONS
{
    address = arg_addr;
    microAddress = arg_micro_address;
    length = arg_length;
    startOfFlow = arg_start_of_flow;
    endOfFlow = arg_end_of_flow;  
    istreamStartup = false;
};

inline
IADDR_CLASS::IADDR_CLASS() //CONS
{
    address = 0;
    microAddress = 0;
    length = 0;
    startOfFlow = 0;
    endOfFlow = 0;
    istreamStartup = false;
};

inline
IADDR_CLASS::IADDR_CLASS(
    const UINT64 arg_addr
 ) //CONS
{
    address = arg_addr;
    microAddress = 0;
    length = 0;
    startOfFlow = 0;
    endOfFlow = 0;
    istreamStartup = false;
};

inline void 
IADDR_CLASS::SetLength(
    unsigned int arg_length)
{
    length = arg_length;
}

inline void 
IADDR_CLASS::Set(
    UINT64 pc)
{
    address = pc;
}

inline void
IADDR_CLASS::SetIstreamStartup()
{
    istreamStartup = true;
}

inline UINT64
IADDR_CLASS::Get(void) const
{
    return address;
}

inline UINT32
IADDR_CLASS::GetLength(void) const
{
    return length;
}
inline void
IADDR_CLASS::SetMacro(const UINT64 arg_addr)
{
    address = arg_addr;
}

inline void
IADDR_CLASS::SetMacro(
    const UINT64 arg_addr,
    const unsigned int arg_length)
{
    address = arg_addr;
    length = arg_length;
}

inline void 
IADDR_CLASS::SetMicro(
    const UINT32 arg_addr)
{
    microAddress = arg_addr;
}

inline void 
IADDR_CLASS::SetEOF(
    const bool arg_EOF)
{
    endOfFlow = arg_EOF;
}

inline void 
IADDR_CLASS::SetSOF(
    const bool arg_SOF)
{
    startOfFlow = arg_SOF;
}


inline UINT64 
IADDR_CLASS::GetMacro() const
{ 
    return address; 
}

inline UINT32 
IADDR_CLASS::GetMicro() const
{
    return microAddress;
}

inline bool 
IADDR_CLASS::GetEOF()
{
    return endOfFlow;
}

inline bool 
IADDR_CLASS::GetSOF()
{
    return startOfFlow;
}

inline bool 
IADDR_CLASS::GetIstreamStartup() const
{
    return istreamStartup;
}

inline void
IADDR_CLASS::Clear()
{
    address = 0;
    length = 0;
}

inline IADDR_CLASS
IADDR_CLASS::Next(void) const
{
    if (endOfFlow)
    {
        return (IADDR_CLASS(address + length, 0, 0, 1, 0)); //CONS
    }
    else
    {
        return (IADDR_CLASS(address, microAddress + 1, length, 0, 0)); //CONS
    }
    
}

inline IADDR_CLASS
IADDR_CLASS::NextMacro(void) const
{
    return (IADDR_CLASS(address + length, 0, 0, 1, 0)); //CONS
}

// output operator
std::ostream & operator << (std::ostream & os, const IADDR_CLASS & ia);

#endif //_IADDR_

//Local Variables:
//pref: "inst_addr.cpp"
//End:
