// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "outlineitem.h"
#include "objfmt.h"
#include "docwriterimpl.h"
#include "docoutlineimpl.h"
#include <core/generic/floatpointtools.h>
#include <core/errlib/errlib.h>

namespace jag {
namespace pdf {

namespace
{
  void output_navigation_key(ObjFmt& fmt, IndirectObjectRef const& obj, Char const* key)
  {
      if (is_valid(obj))
          fmt.dict_key(key).space().ref(obj);
  }
}


OutlineItem::OutlineItem(DocWriterImpl& doc, OutlineRecord const& rec)
    : IndirectObjectImpl(doc)
    , m_outline_rec(rec)
    , m_count(0)
{
}


void OutlineItem::on_output_definition()
{
     ObjFmt& writer = object_writer();
    writer
        .dict_start()
        .dict_key("Title").text_string(m_outline_rec.m_title)
        .dict_key("Parent").space().ref(m_parent)
        .dict_key("Count").space().output(-m_count);

    output_navigation_key(writer, m_prev, "Prev");
    output_navigation_key(writer, m_next, "Next");
    output_navigation_key(writer, m_first, "First");
    output_navigation_key(writer, m_last, "Last");

    // destination
    writer.dict_key("Dest");
    if (m_outline_rec.m_dest_type == OutlineRecord::DEST_STRING)
    {
        m_outline_rec.m_dest->output_object(writer);
    }
    else
    {
        JAG_ASSERT(m_outline_rec.m_dest_type == OutlineRecord::DEST_ID);
        writer
            .space()
            .ref(doc().destination_ref(m_outline_rec.m_destid));
    }

    // style
    if (OutlineStyle const* style = m_outline_rec.m_style)
    {
        JAG_INTERNAL_ERROR_EXPR(doc().version() >= 4);

        if (0 != style->m_style)
        {
            writer
                .dict_key("F")
                .space()
                .output(style->m_style);
        }

        if (!equal_to_zero(style->m_red) ||
             !equal_to_zero(style->m_green) ||
             !equal_to_zero(style->m_blue))
        {
            writer
                .dict_key("C")
                .array_start()
                .output(style->m_red).space()
                .output(style->m_green).space()
                .output(style->m_blue)
                .array_end();
        }

    }

    writer.dict_end();
}


void OutlineItem::set_children(IndirectObjectRef const& first, IndirectObjectRef const& last)
{
    m_first = first;
    m_last = last;
}

void OutlineItem::set_parent_and_siblings(IndirectObjectRef const& parent, IndirectObjectRef const& prev, IndirectObjectRef const& next)
{
    JAG_PRECONDITION(is_valid(parent));

    m_parent = parent;
    m_prev = prev;
    m_next = next;
}




}} // namespace jag::pdf

/** EOF @file */
