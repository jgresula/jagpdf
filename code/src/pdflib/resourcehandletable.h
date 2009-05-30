// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCEHANDLETABLE_H_JG2259__
#define __RESOURCEHANDLETABLE_H_JG2259__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "indirectobjectref.h"
#include "resourcenames.h"
#include <core/generic/noncopyable.h>
#include <core/errlib/errlib.h>
#include <vector>

namespace jag {
namespace pdf {

template<class THANDLE, class RECORD=IndirectObjectRef>
class ResourceHandleTable
    : public noncopyable
{
public:
    typedef THANDLE handle_t;
    typedef RECORD  record_t;

    typedef std::vector<RECORD> ResourceVec;

    RECORD const& resource_record(THANDLE handle) const;
    RECORD& resource_record(THANDLE handle);
    THANDLE register_resource(RECORD const& res_record);
    THANDLE next_handle() const;

    ResourceVec const& storage() const { return m_resources; }

private:
    bool valid_handle(THANDLE handle) const;
    ResourceVec m_resources;
};


//////////////////////////////////////////////////////////////////////////
template<class THANDLE, class RECORD>
THANDLE ResourceHandleTable<THANDLE,RECORD>::register_resource(RECORD const& res_record)
{
    m_resources.push_back(res_record);
    return THANDLE(static_cast<UInt>(m_resources.size()));
}


//////////////////////////////////////////////////////////////////////////
template<class THANDLE, class RECORD>
RECORD const& ResourceHandleTable<THANDLE,RECORD>::resource_record(THANDLE handle) const
{
    if (!valid_handle(handle))
        throw exception_invalid_value(msg_invalid_object_handle()) << JAGLOC;

    return m_resources[index_from_handle(handle)];
}

//////////////////////////////////////////////////////////////////////////
template<class THANDLE, class RECORD>
RECORD& ResourceHandleTable<THANDLE,RECORD>::resource_record(THANDLE handle)
{
    if (!valid_handle(handle))
        throw exception_invalid_value(msg_invalid_object_handle()) << JAGLOC;

    return m_resources[index_from_handle(handle)];
}


//////////////////////////////////////////////////////////////////////////
template<class THANDLE, class RECORD>
bool ResourceHandleTable<THANDLE,RECORD>::valid_handle(THANDLE handle) const
{
    return    handle.id() != THANDLE::INVALID_VALUE
        && handle.id() <= m_resources.size()
    ;
}

//////////////////////////////////////////////////////////////////////////
template<class THANDLE, class RECORD>
THANDLE ResourceHandleTable<THANDLE,RECORD>::next_handle() const
{
    return THANDLE(m_resources.size()+1);
}


}} //namespace jag::pdf


#endif //__RESOURCEHANDLETABLE_H_JG2259__

