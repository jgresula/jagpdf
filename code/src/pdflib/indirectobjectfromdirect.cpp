// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "indirectobjectfromdirect.h"
#include "interfaces/directobject.h"
#include <core/generic/null_deleter.h>
#include <core/generic/assert.h>

namespace jag {
namespace pdf {


IndirectObjectFromDirect::IndirectObjectFromDirect(DocWriterImpl& doc)
  : IndirectObjectImpl(doc)
{
}


IndirectObjectFromDirect::IndirectObjectFromDirect(DocWriterImpl& doc, std::auto_ptr<IDirectObject> obj)
    : IndirectObjectImpl(doc)
    , m_direct(obj.release())
{
}

IndirectObjectFromDirect::IndirectObjectFromDirect(DocWriterImpl& doc, IDirectObject& obj)
    : IndirectObjectImpl(doc)
    , m_direct(&obj, null_deleter)
{
}


IDirectObject* IndirectObjectFromDirect::object() const
{
    return m_direct.get();
}


void IndirectObjectFromDirect::object(std::auto_ptr<IDirectObject> obj)
{
    m_direct.reset(obj.release());
}


void IndirectObjectFromDirect::on_output_definition()
{
    JAG_ASSERT(m_direct);
    m_direct->output_object(object_writer());
}


}} // namespace jag::pdf

/** EOF @file */
