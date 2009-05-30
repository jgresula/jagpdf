// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CROSSREFSECTION_H_JG2351__
#define __CROSSREFSECTION_H_JG2351__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/stdtypes.h>
#include <vector>

namespace jag
{
class ISeqStreamOutput;

namespace pdf
{

/**
 * @brief bookkeeping of indirect objects
 *
 * basic version, can represent only continuous object numbers
 */
class CrossReferenceSection
{
public:
    CrossReferenceSection();
    /// adds an indirect object
    void add_indirect_object(
          Int object_number
        , Int generation_number
        , Int stream_offset
   );

    void output(ISeqStreamOutput& seq_stream);
    int num_entries() const;
    UInt stream_offset() const;

private:
    /// single object entry in crossreference table
    struct Entry
    {
        Int m_generation_number;
        Int m_stream_offset;
        Char  m_type;
        bool        m_valid;

        Entry() : m_valid(false) {}
        Entry(Int generation_number, Int stream_offset, Char type = 'n')
            : m_generation_number(generation_number)
            , m_stream_offset(stream_offset)
            , m_type(type)
            , m_valid(true) {}
    };

    typedef std::vector<Entry> Entries;
    Entries m_objects;
    ULong m_stream_offset;
};



}} //namespace jag::pdf


#endif //__CROSSREFSECTION_H_JG2351__

