// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCETABLE_H_JAG_1410__
#define __RESOURCETABLE_H_JAG_1410__

#include <interfaces/refcounted.h>
#include <core/generic/assert.h>
#include <core/errlib/errlib.h>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive/list.hpp>
#include <vector>

namespace jag {
namespace resources {

///
/// resource <--> handle, 0 is an invalid handle
///
template<class ID, class RES, class CONT=std::vector<RES> >
class ResourceTable
{
public:
    explicit ResourceTable(UInt start_id = 1);
    RES const& lookup(ID res_id) const;
    RES& lookup(ID res_id);
    ID add(typename CONT::value_type const& resource);
    size_t size() const { return m_resources.size(); }

private:
    typedef CONT ResourceVec;
    ResourceVec m_resources;
    const UInt m_compensation;
};



/**
 * @brief Ctor.
 *
 * @param start_id the lowest id that can be issued > 0
 **/
template<class ID, class RES, class CONT>
ResourceTable<ID,RES,CONT>::ResourceTable(UInt start_id)
    : m_compensation(start_id-1)
{
    JAG_PRECONDITION(start_id>0);
}



//////////////////////////////////////////////////////////////////////////
template<class ID, class RES, class CONT>
RES const& ResourceTable<ID,RES,CONT>::lookup(ID res_id) const
{
    res_id.minus(m_compensation);

    if (!is_valid(res_id) || (res_id.id()>m_resources.size()))
        throw exception_invalid_value(msg_invalid_object_handle()) << JAGLOC;

    return m_resources[res_id.id()-1];
}

//
//
//
template<class ID, class RES, class CONT>
RES& ResourceTable<ID,RES,CONT>::lookup(ID res_id)
{
    res_id.minus(m_compensation);

    if (!is_valid(res_id) || (res_id.id()>m_resources.size()))
        throw exception_invalid_value(msg_invalid_object_handle()) << JAGLOC;

    return m_resources[res_id.id()-1];
}




//////////////////////////////////////////////////////////////////////////
template<class ID, class RES, class CONT>
ID ResourceTable<ID,RES, CONT>::add(typename CONT::value_type const& resource)
{
    m_resources.push_back(resource);
    return ID(static_cast<UInt>(m_resources.size())).plus(m_compensation);
}




// ---------------------------------------------------------------------------
//                    class IssuedResDefList
//



//
//  Maintains list of resource definitions that are given to the client.
//
template<class Def>
class IssuedResDefList
{
    typedef boost::intrusive::list<Def> List;
    List m_list;

public:
    typedef typename List::iterator Iterator;

    // it might happen that the client requested a resource definition object
    // but did not loaded it; such objects are in m_list and are released here
    ~IssuedResDefList() {
        m_list.clear_and_dispose(boost::checked_deleter<Def>());
    }

    // searches the list for the given definition, throws an exception if not
    // found (i.e. already looked up or utterly unknown)
    Iterator lookup(Def *resource_def) {
        Iterator it_end = m_list.end();
        Iterator it = m_list.begin();
        for(; it!=it_end; ++it)
        {
            if (&*it == resource_def)
                break;
        }
        if (it == m_list.end())
            throw exception_invalid_operation() << JAGLOC;

        return it;
    }

    // transfer ownership from m_list back to the client
    std::auto_ptr<Def> detach(Iterator it) {
        std::auto_ptr<Def> retval(&*it);
        m_list.erase(it); // does not throw
        return retval;
    }

    // adds a new resource definition to the list - the ownership is transfered
    Def* add(std::auto_ptr<Def> new_one) {
        m_list.push_back(*new_one.release());
        return &m_list.back();
    }
};



}} // namespace jag::resources

#endif //__RESOURCETABLE_H_JAG_1410__

