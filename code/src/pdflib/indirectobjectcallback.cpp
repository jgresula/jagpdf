// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "indirectobjectcallback.h"
#include <core/generic/assert.h>

namespace jag {
namespace pdf {


IndirectObjectCallback::IndirectObjectCallback(
    DocWriterImpl& doc
    , output_callback_t const& on_output
)
    : IndirectObjectImpl(doc)
    , m_on_output(on_output)
{
}



IndirectObjectCallback::IndirectObjectCallback(
      DocWriterImpl& doc
    , output_callback_t const& on_output
    , before_callback_t const& on_before_output
)
    : IndirectObjectImpl(doc)
    , m_on_output(on_output)
    , m_on_before_output(on_before_output)
{
}



void IndirectObjectCallback::on_output_definition()
{
    JAG_PRECONDITION(m_on_output);
    m_on_output(object_writer());
}



bool IndirectObjectCallback::on_before_output_definition()
{
    if (m_on_before_output)
        return m_on_before_output(object_writer());

    return true;
}

}} // namespace jag::pdf

/** EOF @file */
