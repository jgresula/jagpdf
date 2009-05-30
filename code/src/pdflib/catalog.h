// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef CATALOG_H__1822128
#define CATALOG_H__1822128

#include "indirectobjectimpl.h"
#include "page_object.h"
#include "tree_node.h"
#include "indirectobjectref.h"

#include <boost/intrusive_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/spirit/include/classic_symbols_fwd.hpp>

namespace jag { namespace pdf
{

class PDFCatalog
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE
    typedef boost::ptr_vector<PageObject> Pages;
    explicit PDFCatalog(DocWriterImpl& doc);
    ~PDFCatalog() {}
    void add_page(std::auto_ptr<PageObject> page);
    size_t num_pages() const { return m_pages.size(); }
    IndirectObjectRef page_ref(int page_num) const;
    Pages const& pages() const { return m_pages; }

private:
    void initial_page_view();
    void write_dict_option(boost::spirit::classic::symbols<Int> const& symbols,
                           Char const* option,
                           Char const* pdf_kwd);
    void write_viewer_prefs();

private: // IndirectObjectImpl
    void on_output_definition();
    bool on_before_output_definition();

private:
    Pages                                       m_pages;
    IndirectObjectRef                           m_doc_outline;
    IndirectObjectRef                           m_page_tree_root;
    IndirectObjectRef                           m_viewer_prefs;
};

}} //namespace jag::pdf

#endif //CATALOG_H__1822128

