// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "docoutlineimpl.h"
#include "objfmt.h"
#include "docwriterimpl.h"
#include "outlineitem.h"

#include <core/generic/refcountedimpl.h>
#include <core/generic/stringutils.h>
#include <core/errlib/errlib.h>
#include <msg_pdflib.h>
#include <boost/ref.hpp>


using namespace boost;

namespace jag {
namespace pdf {

namespace
{

  const OutlineStyle default_style = { 0 };

  bool is_default_style(OutlineStyle const& style)
  {
      return !memcmp(&default_style, &style, sizeof(OutlineStyle));
  }
}


DocOutlineImpl::DocOutlineImpl(DocWriterImpl& doc)
    : IndirectObjectImpl(doc)
    , m_string_pool(512)
    , m_outlines_depth(0)
    , m_current_style(default_style)
{
}


void DocOutlineImpl::on_output_definition()
{
    ObjFmt& writer = object_writer();
    writer.dict_start().dict_key("Type").output("Outlines");

    JAG_ASSERT(is_valid(m_first));
    writer.dict_key("First").space().ref(m_first);

    JAG_ASSERT(is_valid(m_last));
    writer.dict_key("Last").space().ref(m_last);

    writer.dict_end();
}


bool DocOutlineImpl::on_before_output_definition()
{
    ItemsStack items_stack;
    items_stack.push_back(new Items);  // insert the top level

    const size_t cmds_len = m_outlines.size();
    for(size_t i=0; i<cmds_len; ++i)
    {
        if (m_outlines[i] < 0)
        {
            // move up/down in the outline hierarchy
            if (m_outlines[i] == LEVEL_UP)
            {
                JAG_ASSERT(!items_stack.back().empty());
                pop_outline_level(items_stack);
            }
            else
            {
                JAG_ASSERT(m_outlines[i] == LEVEL_DOWN);
                items_stack.push_back(new Items);
            }
        }
        else
        {
            OutlineRecord const& rec(m_records[m_outlines[i]]);

            // put a bookmark to the current level
            std::auto_ptr<OutlineItem> bookmark(
                new RefCountImpl<OutlineItem>(doc(), rec));

           items_stack.back().push_back(bookmark.release());
        }
    }

    JAG_ASSERT(!items_stack.back().empty());
    while (items_stack.size() != 1)
    {
        // the last up commands not supplied
        pop_outline_level(items_stack);
    }

    pop_outline_level(items_stack);
    return true;
}


/**
 * @brief Finishes one level of the outline hierarchy.
 *
 * @param items_stack   outlines hierarchy
 */
void DocOutlineImpl::pop_outline_level(ItemsStack& items_stack)
{
    const size_t outline_level = items_stack.size();
    const bool top_level =  outline_level == 1;

    IndirectObjectRef parent_ref = top_level
        ? IndirectObjectRef(*this)
        : IndirectObjectRef(items_stack[outline_level-2].back());

    Items& this_level(items_stack[outline_level-1]);
    const size_t num_items = this_level.size();
    JAG_ASSERT(num_items);
    const size_t last_item = num_items-1;


    // link the outlines on the current level and output them
    int num_descendants = 0;
    for(size_t i=0; i<num_items; ++i)
    {
        num_descendants += this_level[i].count();

        IndirectObjectRef prev, next;
        if (i>0)
            prev = IndirectObjectRef(this_level[i-1]);

        if (i<last_item)
            next = IndirectObjectRef(this_level[i+1]);

        this_level[i].set_parent_and_siblings(parent_ref, prev, next);
        this_level[i].output_definition();
    }

    // set first/last references in parent
    if (top_level)
    {
        m_first = IndirectObjectRef(this_level[0]);
        m_last = IndirectObjectRef(this_level[last_item]);
    }
    else
    {
        OutlineItem& parent = items_stack[outline_level-2].back();

        parent.set_children(
            IndirectObjectRef(this_level[0]),
            IndirectObjectRef(this_level[last_item]));
        parent.count(num_descendants);
    }

    items_stack.pop_back();
}


/// Retrieves 'permanent' pointer to the current style.
OutlineStyle* DocOutlineImpl::ptr_to_current_style()
{
    if (doc().version() < 4 || is_default_style(m_current_style))
        return 0;

    OutlineStyle *const p = m_used_styles_pool.construct(m_current_style);
    return p;
}

template<class T>
void DocOutlineImpl::add_outline_rec(Char const* title, T const& dest)
{
    m_records.push_back(
        OutlineRecord(m_string_pool.add(safe_null_string(title)),
                       dest,
                       ptr_to_current_style()));

    m_outlines.push_back(static_cast<int>(m_records.size())-1);
}


void DocOutlineImpl::item(Char const* title)
{
    if (!doc().is_page_opened())
        throw exception_invalid_operation(msg_bookmark_outside_page_def()) << JAGLOC;

    DestinationDef* dest = m_destinations.construct(ref(doc()));
    const int page_num = doc().page_number();
    dest->set_page_num(page_num);

    Double page_y = doc().is_topdown()
        ? 0.0
        : doc().page_height(page_num);
    
    dest->set_xyz(DestinationDef::no_change, page_y);

    add_outline_rec(title, dest);
}


void DocOutlineImpl::item_destination(Char const* title, Char const* dest)
{
    DestinationDef* destp = m_destinations.construct(ref(doc()), dest );
    destp->set_page_num(doc().page_number(), false);

    add_outline_rec(title, destp);
}


void DocOutlineImpl::item_destination_obj(Char const* title, Destination id)
{
    add_outline_rec(title, id);
}


void DocOutlineImpl::level_up()
{
    if (!m_outlines_depth)
        throw exception_invalid_operation() << JAGLOC;

    ++m_outlines_depth;
    m_outlines.push_back(LEVEL_UP);
}

void DocOutlineImpl::level_down()
{
    if (m_outlines.empty())
        throw exception_invalid_operation() << JAGLOC;

    --m_outlines_depth;
    m_outlines.push_back(LEVEL_DOWN);
}




void DocOutlineImpl::color(Double red, Double green, Double blue)
{
    doc().ensure_version(4, "outline item color");
    m_current_style.m_red = red;
    m_current_style.m_green = green;
    m_current_style.m_blue = blue;
}

void DocOutlineImpl::style(Int val)
{
    doc().ensure_version(4, "outline item style");
    m_current_style.m_style = val;
}

void DocOutlineImpl::state_save()
{
    if (doc().version() < 4)
        return;

    m_styles_stack.push_back(m_current_style);
}

void DocOutlineImpl::state_restore()
{
    if (doc().version() < 4)
        return;

    if (m_styles_stack.empty())
        throw exception_invalid_operation() << JAGLOC;

    m_current_style = m_styles_stack.back();
    m_styles_stack.pop_back();
}



}} // namespace jag::pdf

/** EOF @file */
