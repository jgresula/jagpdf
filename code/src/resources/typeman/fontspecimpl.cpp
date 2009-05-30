// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "fontspecimpl.h"
#include <resources/typeman/charencrecord.h>
#include <msg_resources.h>
#include <core/generic/floatpointtools.h>
#include <core/generic/stringutils.h>
#include <core/generic/refcountedimpl.h>
#include <core/jstd/fileso.h>
#include <boost/intrusive_ptr.hpp>

using namespace jag::jstd;
using namespace boost;

namespace jag { namespace resources
{

//////////////////////////////////////////////////////////////////////////
FontSpecImpl::FontSpecImpl()
    : m_pointsize(0.0)
    , m_bold(0)
    , m_italic(0)
    , m_adobe14(false)
{
}


//////////////////////////////////////////////////////////////////////////
void FontSpecImpl::ensure_consistency() const
{
    if (m_facename.empty() && m_filename.empty())
        throw exception_invalid_value(msg_face_or_name_must_be_set()) << JAGLOC;

    if (!m_facename.empty() && !m_filename.empty())
        throw exception_invalid_value(msg_face_or_name()) << JAGLOC;

    // point size must be set
    if (equal_to_zero(m_pointsize))
        throw exception_invalid_value(msg_font_size_not_set()) << JAGLOC;


    // do not report non-existing file, as we might try to load a
    // default typeface
}


//
//
//
void FontSpecImpl::facename(Char const* val)
{
    if (!is_empty(val))
    {
        m_facename=val;
    }
    else
    {
        m_facename.clear();
    }
}

//////////////////////////////////////////////////////////////////////////
void FontSpecImpl::filename(Char const* val)
{
    m_filename.assign(val);
}

//////////////////////////////////////////////////////////////////////////
void FontSpecImpl::filename(str_t const& str)
{
    m_filename.assign(str.first, str.second);
}

//////////////////////////////////////////////////////////////////////////
void FontSpecImpl::filename2(str_t const& str)
{
    m_filename2.assign(str.first, str.second);
}

//////////////////////////////////////////////////////////////////////////
void FontSpecImpl::facename(str_t const& str)
{
    std::string(str.first, str.second).swap(m_facename);
}

//////////////////////////////////////////////////////////////////////////
std::string FontSpecImpl::fullname() const
{
    JAG_PRECONDITION(!m_facename.empty());
    std::string result(m_facename);
    if (m_bold)
        result += " Bold";

    if (m_italic)
        result += " Italic";

    if (!m_bold && !m_italic)
        result += " Regular";

    return result;
}


//////////////////////////////////////////////////////////////////////////
void FontSpecImpl::encoding(str_t const& str)
{
    m_encoding.assign(str.first, str.second);
}

//////////////////////////////////////////////////////////////////////////
void FontSpecImpl::encoding(Char const* val)
{
    m_encoding = val;
}

//
//
//
boost::intrusive_ptr<FontSpecImpl> FontSpecImpl::clone() const
{
    intrusive_ptr<FontSpecImpl> clone(new RefCountImpl<FontSpecImpl>());

    clone->m_facename = m_facename;
    clone->m_encoding = m_encoding;
    clone->m_filename = m_filename;
    clone->m_filename2 = m_filename2;
    clone->m_pointsize = m_pointsize;
    clone->m_bold = m_bold;
    clone->m_italic = m_italic;
    clone->m_adobe14 = m_adobe14;

    return clone;
}



}} //namespace jag::resources

