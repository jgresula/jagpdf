// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "typefacespecimpl.h"
#include <core/generic/assert.h>
#include <core/jstd/memory_stream.h>
#include <msg_resources.h>

namespace jag { namespace resources
{

/// ensures that the spec is complete and can be registered
void TypefaceSpecImpl::ensure_consistency() const
{
    if (m_file_name.empty() && !m_font_data)
        throw exception_invalid_value(msg_typeface_data_not_specified()) << JAGLOC;
}

//////////////////////////////////////////////////////////////////////////
void TypefaceSpecImpl::data(Byte const* array, UInt length)
{
    if (!m_font_data)
        m_font_data.reset(new jstd::MemoryStreamOutput);

    m_font_data->write(array, length);
}


/// determines location of font data
TypefaceSpecImpl::TypefaceSource TypefaceSpecImpl::source() const
{
    if (m_font_data)
        return IN_MEMORY;

    JAG_ASSERT(!m_file_name.empty());
    return IN_FILE;
}


/// retrieves pointer to font data
SharedArray TypefaceSpecImpl::memory_data() const
{
    JAG_PRECONDITION(IN_MEMORY==source());
    return SharedArray(
          m_font_data->shared_data()
        , static_cast<size_t>(m_font_data->tell())
   );
}

}} //namespace jag::resources
