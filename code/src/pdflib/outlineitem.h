// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef OUTLINEITEM_JG1310_H__
#define OUTLINEITEM_JG1310_H__

#include "indirectobjectimpl.h"
#include "indirectobjectref.h"

namespace jag {
namespace pdf {

struct OutlineRecord;

/**
 * @brief Represents a single outline item in the bookmark tree.
 *
 * Lifetime of this object is always bound by DocOutlineImpl
 * object. In particular it is created (and released) during
 * DocOutlineImpl output.
 *
 * This enables keeping some state (OutlineRecord) only by reference
 * since that state is owned by DocOutlineImpl and its lifetime always
 * exceeds lifetime of this object.
 */
class OutlineItem
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE
    OutlineItem(DocWriterImpl& doc, OutlineRecord const& rec);

public:
    void set_children(IndirectObjectRef const& first, IndirectObjectRef const& last);
    void set_parent_and_siblings(IndirectObjectRef const& parent, IndirectObjectRef const& prev, IndirectObjectRef const& next);
    int count() const { return m_count; }
    void count(int cnt) { m_count = cnt; }

private: // IndirectObjectImpl
    void on_output_definition();

private:
    OutlineRecord const&  m_outline_rec;
    int                   m_count;
    IndirectObjectRef     m_first;
    IndirectObjectRef     m_last;
    IndirectObjectRef     m_parent;
    IndirectObjectRef     m_prev;
    IndirectObjectRef     m_next;
};


}} // namespace jag::pdf

#endif // OUTLINEITEM_JG1310_H__
/** EOF @file */
