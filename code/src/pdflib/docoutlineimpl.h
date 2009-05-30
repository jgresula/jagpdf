// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef DOCOUTLINEIMPL_JG2249_H__
#define DOCOUTLINEIMPL_JG2249_H__

#include "indirectobjectimpl.h"
#include "indirectobjectref.h"
#include "destination.h"

#include <pdflib/interfaces/docoutline.h>
#include <core/jstd/stringpool.h>


#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/pool/object_pool.hpp>
#include <deque>
#include <vector>

namespace jag {
namespace pdf {

class DocWriterImpl;
class OutlineItem;

struct OutlineStyle
{
    Int  m_style;
    Double m_red;
    Double m_green;
    Double m_blue;
};


/// Holds destination, title and a style for an outline entry
struct OutlineRecord
{
    enum DestinationType { DEST_STRING, DEST_ID };
    OutlineRecord(Char const* title, DestinationDef* dest, OutlineStyle const* style=0)
        : m_title(title)
        , m_style(style)
        , m_dest_type(DEST_STRING)
        , m_dest(dest)
    {}

    OutlineRecord(Char const* title, Destination destid, OutlineStyle const* style=0)
        : m_title(title)
        , m_style(style)
        , m_dest_type(DEST_ID)
        , m_destid(destid)
    {}


    Char const*              m_title;
    OutlineStyle const*      m_style;

    DestinationType          m_dest_type;
    union
    {
        DestinationDef*         m_dest;
        Destination        m_destid;
    };
};


/**
 * @brief Implements interface allowing to specify bookmarks.
 */
class DocOutlineImpl
    : public IndirectObjectImpl
    , public IDocumentOutline
{
public:
    DEFINE_VISITABLE
    explicit DocOutlineImpl(DocWriterImpl& doc);

public:
    bool is_empty() { return m_outlines.empty(); }

public:
    void item(Char const* title);
    void level_up();
    void level_down();
    void color(Double red, Double green, Double blue);
    void style(Int val);
    void state_save();
    void state_restore();
    void item_destination(Char const* title, Char const* dest);
    void item_destination_obj(Char const* title, Destination id);


private: // IndirectObjectImpl
    void on_output_definition();
    bool on_before_output_definition();

private:
    typedef boost::ptr_vector<OutlineItem> Items;
    typedef boost::ptr_deque<Items>        ItemsStack;

    void pop_outline_level(ItemsStack& items_stack);
    OutlineStyle* ptr_to_current_style();
    template<class T>
    void add_outline_rec(Char const* title, T const& dest);

private:
    // some data is kept in a memory pools - namely the data
    // referenced from OutlineRecord
    enum ActionEnum { LEVEL_UP=-1, LEVEL_DOWN=-2 };

    jstd::StringPool              m_string_pool;
    std::vector<int>                 m_outlines;
    std::vector<OutlineRecord>       m_records;
    int                              m_outlines_depth;
    boost::object_pool<DestinationDef>  m_destinations;

    // references to the first and list child
    IndirectObjectRef           m_first;
    IndirectObjectRef           m_last;

    // outline entry style related
    OutlineStyle                     m_current_style;
    boost::object_pool<OutlineStyle> m_used_styles_pool;
    std::deque<OutlineStyle>         m_styles_stack;
};


}} // namespace jag::pdf

#endif // DOCOUTLINEIMPL_JG2249_H__
/** EOF @file */
