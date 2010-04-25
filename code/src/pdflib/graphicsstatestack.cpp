// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "graphicsstatestack.h"
#include <core/errlib/errlib.h>
#include <core/generic/assert.h>
#include <msg_pdflib.h>

namespace jag {
namespace pdf {

/// ctor
GraphicsStateStack::GraphicsStateStack(
      DocWriterImpl& doc
    , ObjFmtBasic& fmt
)
    : m_doc(doc)
    , m_fmt(fmt)
    , m_last_commited(m_doc)
{
    m_stack.push_back(m_last_commited);
}

//////////////////////////////////////////////////////////////////////////
GraphicsStateHandle GraphicsStateStack::save()
{
    // two important notes here
    // - the top is always committed before pushing another item
    //    it ensures that all the time there is at most one uncommitted
    //    item and if so it is the top
    // - copy of the current top is pushed, which enables managing
    //    of incremental output of graphics state efficiently
    GraphicsStateHandle result(commit());
    m_stack.push_back(m_stack.back());
    return result;
}

//////////////////////////////////////////////////////////////////////////
void GraphicsStateStack::restore()
{
    if (m_stack.size() == 1)
        throw exception_invalid_operation(msg_no_graphics_state_to_pop()) << JAGLOC;

    m_stack.pop_back();
    m_last_commited = m_stack.back();
}

//////////////////////////////////////////////////////////////////////////
GraphicsStateHandle GraphicsStateStack::commit()
{
    if (!m_stack.back().is_committed())
    {
        if (!m_last_commited.is_equal_state(m_stack.back()))
        {
            m_last_commited = m_stack.back();
            return m_stack.back().commit(m_fmt);
        }
    }

    return GraphicsStateHandle();
}

//////////////////////////////////////////////////////////////////////////
GraphicsState& GraphicsStateStack::top()
{
    JAG_PRECONDITION(!m_stack.empty());
    return m_stack.back();
}

//
//
// 
GraphicsState const& GraphicsStateStack::top() const
{
    JAG_PRECONDITION(!m_stack.empty());
    return m_stack.back();
}


}} //namespace jag::pdf
