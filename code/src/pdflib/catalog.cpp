// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "catalog.h"
#include "objfmt.h"
#include "page_tree.h"
#include "docwriterimpl.h"
#include "destination.h"
#include "generic_dictionary.h"
#include "genericcontentstream.h"
#include <core/jstd/tracer.h>
#include <boost/spirit/include/classic_symbols.hpp>
#include <boost/bind.hpp>
#include <interfaces/execcontext.h>
#include <interfaces/configinternal.h>
#include <core/errlib/except.h>
#include <core/jstd/tracer.h>
#include <core/jstd/optionsparser.h>
#include <msg_jstd.h>
#include <core/jstd/streamhelpers.h>

using namespace boost;
using namespace jag::jstd;

namespace jag {
namespace pdf {

namespace
{
  //
  //
  //
  void output_dict_ref(ObjFmt& writer,
                       IndirectObjectRef const& dict,
                       Char const* kwd)
  {
      if (is_valid(dict))
          writer.dict_key(kwd).space().ref(dict);
  }

  void output_icc_dict(int ncomponents, ObjFmt& fmt)
  {
      // callback - enriches stream's dict by iccbased color space specifics
      fmt.dict_key("N").space().output(ncomponents);
  }
  
  void output_icc_profile(DocWriterImpl& doc, output_intent_t& intent)
  {
      // output the stream with icc profile
      GenericContentStream cstream(
          doc
          , bind(&output_icc_dict, intent.ncomponents, _1));
      
      copy_stream(*intent.icc_stream, cstream.out_stream());
      intent.profile_ref = IndirectObjectRef(cstream);
      cstream.output_definition();
  }
}

//////////////////////////////////////////////////////////////////////////
PDFCatalog::PDFCatalog(DocWriterImpl& doc)
    : IndirectObjectImpl(doc)
{
}


//////////////////////////////////////////////////////////////////////////
void PDFCatalog::on_output_definition()
{
    ObjFmt& writer = object_writer();

    writer.dict_start()
        .dict_key("Type").output("Catalog")
    ;

    JAG_ASSERT(is_valid(m_page_tree_root));

    output_dict_ref(writer, m_page_tree_root, "Pages");
    output_dict_ref(writer, m_doc_outline, "Outlines");
    output_dict_ref(writer, m_viewer_prefs, "ViewerPreferences");

    initial_page_view();
    write_output_intent();
    writer.dict_end();
}

//
//
// 
void PDFCatalog::write_output_intent()
{
    if (!m_output_intent)
        return;
    
    ObjFmt& fmt = object_writer();

    fmt
        .dict_key("OutputIntents")
        .array_start()
        .dict_start()
        .dict_key("S").name("GTS_PDFX")
        .dict_key("OutputConditionIdentifier")
        .text_string(m_output_intent->output_condition_id);

    if (!m_output_intent->info.empty())
        fmt.dict_key("Info").text_string(m_output_intent->info);

    if (!m_output_intent->output_condition.empty())
        fmt.dict_key("OutputCondition").text_string(m_output_intent->output_condition);

    if (is_valid(m_output_intent->profile_ref))
        fmt.dict_key("DestOutputProfile").space().ref(m_output_intent->profile_ref);
        
    fmt
        .dict_end()
        .array_end();
}

namespace
{
  //
  // Keywords for PageMode and PageLayout options. The value
  // is minimal required PDF version.
  //
  struct PageLayoutKeywords
      : public spirit::classic::symbols<Int>
  {
      PageLayoutKeywords()
      {
          add
              ("SinglePage", 0)
              ("OneColumn", 0)
              ("TwoColumnLeft", 0)
              ("TwoColumnRight", 0)
              // Preflight reports these two options as unexpected values, but
              // it seems that displays them as specified. Let's disable it for
              // now until we know more
//               ("TwoPageLeft", 5)
//               ("TwoPageRight", 5)
              ;
      }
  } g_page_layout_kwds;

  struct PageModeKeywords
      : public spirit::classic::symbols<Int>
  {
      PageModeKeywords()
      {
          add
              ("UseNone", 0)
              ("UseOutlines", 0)
              ("UseThumbs", 0)
              ("FullScreen", 0);
      }
  } g_page_mode_kwds;


  //
  // Keywords for ViewerPreferences options
  //
  enum {HIDE_TOOLBAR, HIDE_MENUBAR, HIDE_WINDOW_UI, \
        FIT_WINDOW, CENTER_WINDOW, DISPLAY_DOC_TITLE, NUM_VIEWER_PREFS};

  char const*const g_viewer_prefs_kwds_str[NUM_VIEWER_PREFS] = {
      "HideToolbar",
      "HideMenubar",
      "HideWindowUI",
      "FitWindow",
      "CenterWindow",
      "DisplayDocTitle"
  };

  struct ViewerPreferencesKeywords
      : public spirit::classic::symbols<unsigned>
  {
      ViewerPreferencesKeywords()
      {
          add
              (g_viewer_prefs_kwds_str[HIDE_TOOLBAR], HIDE_TOOLBAR)
              (g_viewer_prefs_kwds_str[HIDE_MENUBAR], HIDE_MENUBAR)
              (g_viewer_prefs_kwds_str[HIDE_WINDOW_UI], HIDE_WINDOW_UI)
              (g_viewer_prefs_kwds_str[FIT_WINDOW], FIT_WINDOW)
              (g_viewer_prefs_kwds_str[CENTER_WINDOW], CENTER_WINDOW)
              (g_viewer_prefs_kwds_str[DISPLAY_DOC_TITLE], DISPLAY_DOC_TITLE);
      }
  } g_viewer_prefs_kwds;

}   // end anonymous namespacw

//
// A generic routine writing PageMode and PageLyout options.
//
void PDFCatalog::write_dict_option(spirit::classic::symbols<Int> const& symbols,
                                   Char const* option,
                                   Char const* pdf_kwd)
{
    IProfileInternal const& profile = doc().exec_context().config();
    Char const* value = profile.get(option);

    if (is_empty(value))
        return;

    Int* version = find(symbols, value);
    if (!version)
    {
        throw exception_invalid_value(
            msg_config_unknown_value(option, value)) << JAGLOC;
    }
    else if (*version <= doc().version())
    {
        object_writer().dict_key(pdf_kwd).output(value);
    }
    else
    {
        TRACE_WRN << '\'' << value << "' is available since PDF 1." << *version << '.';
    }
}


//
//
//
void PDFCatalog::initial_page_view()
{
    // TBD: share exec_ctx, profile and writer among all subroutines

    write_dict_option(g_page_layout_kwds, "doc.page_layout", "PageLayout");
    write_dict_option(g_page_mode_kwds, "doc.page_mode", "PageMode");

    IProfileInternal const& profile = doc().exec_context().config();

    // initial destination
    Char const* initial_dest = profile.get("doc.initial_destination");
    if (!is_empty(initial_dest))
    {
        DestinationDef dest(doc(), initial_dest);
        dest.set_page_num(0, false);
        ObjFmt& writer = object_writer();
        writer.dict_key("OpenAction");
        dest.output_object(writer);
    }
}


//
// Outputs viewer preferences as specified in the profile.
//
void PDFCatalog::write_viewer_prefs()
{
    IProfileInternal const& profile = doc().exec_context().config();

    Char const* viewer_prefs = profile.get("doc.viewer_preferences");
    if (is_empty(viewer_prefs))
        return;

    ParsedResult const& p =
        parse_options(viewer_prefs, ParseArgs(0, &g_viewer_prefs_kwds));

    GenericDictionary dict(doc());

    for(unsigned i=0; i<NUM_VIEWER_PREFS; ++i)
    {
        if (p.has_explicit_value(i))
        {
            if (i == DISPLAY_DOC_TITLE && doc().version() < 4)
                TRACE_WRN << "'DisplayDocTitle' is available since PDF 1.4";
            else
                dict.insert_bool(g_viewer_prefs_kwds_str[i], true);
        }
    }

    if (!dict.empty())
    {
        dict.output_definition();
        m_viewer_prefs.reset(dict);
    }
}


//////////////////////////////////////////////////////////////////////////
void ProcessPageTreeNode(TreeNode::TreeNodePtr& node)
{
    std::for_each(node->kids_begin(), node->kids_end(), ProcessPageTreeNode);
    node->output_definition();

    /// TBD process inheritable attributes
}

//////////////////////////////////////////////////////////////////////////
bool PDFCatalog::on_before_output_definition()
{
    TRACE_INFO << "Writing pages." ;
    PageTreeBuilder pg_tree_builder(doc(), 3);
    TreeNode::TreeNodePtr tree_root = pg_tree_builder.BuildPageTree(m_pages);
    ProcessPageTreeNode(tree_root);
    m_page_tree_root =  IndirectObjectRef(*tree_root);



    // output document outline (if any)
    IIndirectObject* outline(doc().doc_outline_if_nonempty());
    if (outline)
    {
        TRACE_INFO << "Writing bookmarks." ;
        outline->output_definition();
        m_doc_outline = IndirectObjectRef(*outline);
    }

    // viewer preferences
    write_viewer_prefs();


    // output intent
    if (m_output_intent && m_output_intent->icc_stream)
        output_icc_profile(doc(), *m_output_intent);

    return true;
}


//////////////////////////////////////////////////////////////////////////
void PDFCatalog::add_page(std::auto_ptr<PageObject> page)
{
    m_pages.push_back(page.release());
}


//
//
//
IndirectObjectRef PDFCatalog::page_ref(int page_num) const
{
    JAG_PRECONDITION(page_num < static_cast<int>(num_pages()));
    return IndirectObjectRef(m_pages[page_num]);
}

void PDFCatalog::add_output_intent(std::auto_ptr<output_intent_t> intent)
{
    m_output_intent.reset(intent.release());
}


}} //namespace jag::pdf
