// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "docwriterimpl.h"
#include "canvasimpl.h"
#include "objfmt.h"
#include "pdffile_trailer.h"
#include "crossrefsection.h"
#include "catalog.h"
#include "standard_security_handler.h"
#include "encryption_stream.h"
#include "defines.h"
#include "contentstream.h"
#include "resourcemanagement.h"
#include "fontmanagement.h"
#include "pdffont.h"
#include "interfaces/indirect_object.h"
#include "interfaces/security_handler.h"
#include "docoutlineimpl.h"
#include "indirectobjectfromdirect.h"
#include "interfaces/fontadapter.h"

#include <resources/resourcebox/colorspacehelpers.h>
#include <resources/interfaces/resourcectx.h>
#include <resources/interfaces/imageman.h>
#include <resources/interfaces/imagedata.h>
#include <resources/interfaces/typeman.h>
#include <resources/interfaces/typeface.h>
#include <resources/interfaces/fontinfo.h>
#include <resources/interfaces/colorspaces.h>
#include <resources/interfaces/colorspaceman.h>
#include <resources/interfaces/imagespec.h>
#include <resources/interfaces/imagemask.h>
#include <resources/resourcebox/resourcectxfactory.h>

#include <core/jstd/crt.h>
#include <core/jstd/message_sink_console.h>
#include <core/jstd/md5.h>
#include <core/jstd/unicode.h>
#include <core/jstd/tracer.h>
#include <core/jstd/icumain.h>
#include <core/jstd/file_stream.h>
#include <core/generic/macros.h>
#include <core/generic/refcountedimpl.h>
#include <core/errlib/errlib.h>
#include <msg_pdflib.h>
#include <core/errlib/msg_writer.h>

#include <interfaces/streams.h>
#include <interfaces/configinternal.h>
#include <core/jstd/execcontextimpl.h>
#include <pdflib/cfgsymbols.h>

#include <jagpdf/detail/c_prologue.h>

#include <iostream>
#include <memory>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_map.hpp>


using namespace boost;
using namespace jag::resources;
using namespace jag::jstd;

namespace jag {
namespace pdf {

namespace
{

 /**
  * @brief writes pdf header
  *
  * @param seq_stream  where to write the header
  * @param version     PDF format version being created
  */
  void output_pdf_file_header(ISeqStreamOutput& seq_stream, unsigned int version)
  {
      const int buffer_length = 25;
      Char buffer[buffer_length];
      const unsigned char binary_tag[5] = { 151+'A', 151+'N', 151+'N', 151+'A', 0 };
      int written = jstd::snprintf(buffer, buffer_length, "%%PDF-1.%u\n%%%s\n", version, binary_tag);
      seq_stream.write(buffer, written);
  }


  ///
  class FontAdapter
      : public IFontAdapter
  {
  protected:
      PDFFont const& m_pdf_font;
      IFontEx const* m_other;

  public: // IFont
      Int is_bold() const { return m_other->is_bold(); }
      Int is_italic() const { return m_other->is_italic(); }
      Double size() const { return m_other->size(); }
      Char const* family_name() const { return m_other->family_name(); }
      Char const* style_name() const { return m_other->style_name(); }
      Double advance(Char const* txt_u) const { return m_other->advance(txt_u); }
      Double advance_r(jag::Char const* start, jag::Char const* end) const { return m_other->advance_r(start, end);}
      Double glyph_width(UInt16 glyph_index) const {
          return m_other->glyph_width(glyph_index); }
      Double height() const { return m_other->height(); }
      Double ascender() const { return m_other->ascender(); }
      Double descender() const { return m_other->descender(); }
      Double bbox_xmin() const { return m_other->bbox_xmin(); }
      Double bbox_ymin() const { return m_other->bbox_ymin(); }
      Double bbox_xmax() const { return m_other->bbox_xmax(); }
      Double bbox_ymax() const { return m_other->bbox_ymax(); }

  public: //IFontAdapter
      PDFFont const& font() const {return m_pdf_font;}

  public:
      FontAdapter(PDFFont const& pdf_font)
          : m_pdf_font(pdf_font)
          , m_other(pdf_font.font())
      {}
  };

  /// Encoding adapter for IFont
  class FontInfoReencoder
      : public FontAdapter
  {
      shared_ptr<UConverter> m_from_conv;
      shared_ptr<UConverter> m_to_conv;

  public:
      FontInfoReencoder(PDFFont const& pdf_font, char const* enc)
          : FontAdapter(pdf_font)
          , m_from_conv(create_uconverter(enc))
          , m_to_conv(create_uconverter(m_other->encoding_canonical()))
      {}

  public: // IFont
      Double advance(Char const* txt_u) const
      {
          std::vector<UChar> uchars;
          const std::size_t in_len = strlen(txt_u)+1; // includes terminating 0
          to_unicode(m_from_conv.get(), txt_u, txt_u+in_len, uchars);
          JAG_ASSERT(uchars[uchars.size()-1] == 0);

          std::vector<Char> chars;
          from_unicode(m_to_conv.get(), &uchars[0], &uchars[0]+uchars.size(), chars);

          JAG_ASSERT(chars[chars.size()-1] == 0);
          return FontAdapter::advance(&chars[0]);
      }
  };



} // namespace anonymous




//////////////////////////////////////////////////////////////////////////
// DocWriterImpl state
//////////////////////////////////////////////////////////////////////////
struct DocWriterImpl::DocWriterImpl_
{
public:
    /// ctor
    DocWriterImpl_(
        shared_ptr<ISeqStreamOutputControl> out_stream
        , intrusive_ptr<IProfileInternal> config
   )
        : m_utf8_to_16be("UTF-8", "UTF-16BE")
        , m_out_stream(out_stream)
        , m_last_object_nr(0)
        , m_exec_context(*config)
        , m_num_stream_filters(0)
        , m_default_font(0)
    {
        memset(m_file_id, 0, sizeof(m_file_id));
    }

    /// all members which needs DocWriterImpl in their constructors
    void final_construct(DocWriterImpl& doc)
    {
        m_object_formatter.reset(
            new ObjFmt(*m_out_stream, &doc, doc.m_pimpl->m_utf8_to_16be));

        m_catalog.reset(new PDFCatalog(doc));
        m_trailer.reset(new PDFFileTrailer(doc));
        m_resource_management.reset(new ResourceManagement(doc));
        m_resctx = resources::create_default_resource_ctx();
    }

public: //data
    shared_ptr<IMessageSink>              m_message_sink;
    jag::jstd::MessageSinkConsole      m_message_sink_console;
    UnicodeConverterStream                m_utf8_to_16be;
    shared_ptr<ISeqStreamOutputControl>   m_out_stream;
    UInt                                m_last_object_nr;
    scoped_ptr<ObjFmt>                    m_object_formatter;
    Int                                   m_version;
    DocWriterImpl::FileID                 m_file_id;
    ExecContextImpl                       m_exec_context;
    bool                                  m_static_file_id;
    bool                                  m_flatted_streams;

    enum { MAX_STREAM_FILTERS = 1 };
    StreamFilter                          m_stream_filters[MAX_STREAM_FILTERS];
    int                                   m_num_stream_filters;

    scoped_ptr<PDFCatalog>                m_catalog;
    scoped_ptr<PDFFileTrailer>            m_trailer;
    CrossReferenceSection                 m_cross_reference_section;
    scoped_ptr<DocOutlineImpl>            m_doc_outline;
    ptr_vector<IndirectObjectFromDirect>  m_destinations;

    // page scope related
    std::auto_ptr<PageObject>             m_current_page;

    scoped_ptr<ISecurityHandler>            m_security_handler;
    scoped_ptr<EncryptionStream>            m_encryption_stream;

    // resources
    shared_ptr<IResourceCtx>                m_resctx;
    scoped_ptr<ResourceManagement>          m_resource_management;

    IFont*                              m_default_font;

    // collector
    typedef std::map<PDFFont const*, shared_ptr<IFontAdapter> > FontMap;
    FontMap m_font_map;
    bool m_is_topdown;
};



//////////////////////////////////////////////////////////////////////////
// DocWriterImpl definition
//////////////////////////////////////////////////////////////////////////
DocWriterImplBaseInit::DocWriterImplBaseInit(IProfileInternal& cfg)
{
#   ifndef JAG_DEBUG
    // do not check config keyword typos in release mode
    cfg.verification_active(false);
#   endif
    jag_error_reset();
    set_tracing_level(static_cast<TracingLevels>(cfg.get_int("doc.trace_level")));
    show_trace_location(cfg.get_int("doc.trace_show_loc"));

    TRACE_INFO   << "---Document object created.";
}

DocWriterImplBaseInit::~DocWriterImplBaseInit()
{
    TRACE_INFO << "---Document object released.";
}


DocWriterImpl::DocWriterImpl(
      shared_ptr<ISeqStreamOutputControl> out_stream
    , intrusive_ptr<IProfileInternal> config
)
    : DocWriterImplBaseInit(*config)
    , m_pimpl(new DocWriterImpl_(out_stream, config))
{
    m_pimpl->final_construct(*this);

    // read the configuration
    m_pimpl->m_version = config->get_int("doc.version");
    if (m_pimpl->m_version < 2 || m_pimpl->m_version > 6)
        throw exception_invalid_value(msg_option_out_of_range("doc.version")) << JAGLOC;

    // various flags
    m_pimpl->m_static_file_id = config->get("doc.static_file_id") ? true : false;
    m_pimpl->m_is_topdown = config->get_int("doc.topdown");

    // encoding
    if (config->get_int("doc.compressed"))
        m_pimpl->m_stream_filters[m_pimpl->m_num_stream_filters++] = STREAM_FILTER_FLATE;

    JAG_ASSERT(m_pimpl->m_num_stream_filters <= DocWriterImpl_::MAX_STREAM_FILTERS);


    // encryption
    if (!strcmp(config->get("doc.encryption"), "standard"))
    {
        m_pimpl->m_security_handler.reset(new StandardSecurityHandler(*this));
        m_pimpl->m_encryption_stream.reset(new EncryptionStream(*out_stream, *m_pimpl->m_security_handler));
        m_pimpl->m_object_formatter->encryption_stream(m_pimpl->m_encryption_stream.get());
    }

    output_pdf_file_header(*m_pimpl->m_out_stream, m_pimpl->m_version);

    TRACE_DETAIL << "Document object initialization done.";
}


//////////////////////////////////////////////////////////////////////////
DocWriterImpl::~DocWriterImpl()
{
    try
    {
#ifdef _MSCVER
#    pragma message(__FILE__ " issue warning that the document has not been finalized")
#endif
        m_pimpl.reset();
    }
    catch(std::exception& /*exc*/)
    {
        // log the exception
        JAG_INVARIANT(!"exception caught when releasing pimpl");
    }
}


//////////////////////////////////////////////////////////////////////////
void DocWriterImpl::page_start(Double width, Double height)
{
    TRACE_INFO << "--Page " << page_number()+1 << " started.";
    if (m_pimpl->m_current_page.get())
        throw exception_invalid_operation(msg_page_already_started()) << JAGLOC;

    m_pimpl->m_current_page.reset(new PageObject(*this));
    m_pimpl->m_current_page->set_dimension(width, height);
}


//////////////////////////////////////////////////////////////////////////
void DocWriterImpl::page_end()
{
    if (!m_pimpl->m_current_page.get())
        throw exception_invalid_operation(msg_page_not_started()) << JAGLOC;

    m_pimpl->m_current_page->page_end();
    m_pimpl->m_catalog->add_page(m_pimpl->m_current_page);
    m_pimpl->m_current_page.reset(0);

    TRACE_INFO << "--Page " << page_number() << " done.";
}


void DocWriterImpl::output_destinations()
{
    if (!m_pimpl->m_destinations.empty())
    {
        TRACE_INFO << "Writing destinations.";
        for (size_t i=m_pimpl->m_destinations.size(); i--; )
        {
            if (m_pimpl->m_destinations[i].object())
            {
                m_pimpl->m_destinations[i].output_definition();
            }
            else
            {
                //we could be weaker here: report only referenced but
                //not defined object; now *all* undefined object are
                //reported
                throw exception_invalid_value(msg_destination_not_defined(static_cast<Int>(i))) << JAGLOC;
            }
        }
    }
 }


//
//
//
void DocWriterImpl::finalize()
{
    TRACE_INFO << "--Document object finalization.";
    JAG_ASSERT(m_pimpl);

    if (m_pimpl->m_catalog->num_pages())
    {
        output_destinations();
        m_pimpl->m_catalog->output_definition();

        if (m_pimpl->m_security_handler)
            m_pimpl->m_security_handler->indirect_object().output_definition();

        res_mgm().finalize();
        m_pimpl->m_trailer->before_output();
        m_pimpl->m_cross_reference_section.output(*m_pimpl->m_out_stream);
        m_pimpl->m_trailer->output(
            m_pimpl->m_cross_reference_section.stream_offset(),
            m_pimpl->m_cross_reference_section.num_entries(),
            *m_pimpl->m_catalog,
            m_pimpl->m_security_handler
            ? &m_pimpl->m_security_handler->indirect_object()
            : static_cast<IIndirectObject*>(0));
    }
    else
    {
        write_message(WRN_EMPTY_DOCUMENT);
    }

    m_pimpl->m_object_formatter->flush();
    m_pimpl->m_out_stream->flush();

    // future: the client could pass either stream or stream_ctrl
    // if stream_ctr is available then close is called on it
//    if (m_stream_control)
//        m_stream_control->close();
    m_pimpl->m_out_stream->close();

    // from this point onward only Dispose method can be invoked
    //m_pimpl.reset();

    TRACE_DETAIL << "Document object finalization done.";
}


///////////////////////////////////////////////////////////////////////////
IPage* DocWriterImpl::page()
{
    if (!m_pimpl->m_current_page.get())
        throw exception_invalid_operation() << JAGLOC;

    //REF
    return m_pimpl->m_current_page.get();
}


//////////////////////////////////////////////////////////////////////////
IDocumentOutline* DocWriterImpl::outline()
{
    if (!m_pimpl->m_doc_outline)
        m_pimpl->m_doc_outline.reset(new DocOutlineImpl(*this));

    //REF
    return m_pimpl->m_doc_outline.get();
}


/**
 * @brief Retrieves document outline object if any or non-empty.
 *
 * @return Document outline object; can be 0 if no outlines were created.
 */
IIndirectObject* DocWriterImpl::doc_outline_if_nonempty() const
{
    if (m_pimpl->m_doc_outline && !m_pimpl->m_doc_outline->is_empty())
        return m_pimpl->m_doc_outline.get();

    return 0;
}


//////////////////////////////////////////////////////////////////////////
IMessageSink& DocWriterImpl::message_sink()
{
    if (m_pimpl->m_message_sink)
        return *m_pimpl->m_message_sink;

    return m_pimpl->m_message_sink_console;
}


//////////////////////////////////////////////////////////////////////////
UInt DocWriterImpl::assign_next_object_number()
{
    return ++m_pimpl->m_last_object_nr;
}


//////////////////////////////////////////////////////////////////////////
void DocWriterImpl::add_indirect_object(
      Int object_number
    , Int generation_number
    , Int stream_offset)
{
    m_pimpl->m_cross_reference_section.add_indirect_object(
        object_number,
        generation_number,
        stream_offset);
}


//////////////////////////////////////////////////////////////////////////
ObjFmt& DocWriterImpl::object_writer() const
{
    return *m_pimpl->m_object_formatter;
}


//////////////////////////////////////////////////////////////////////////
void DocWriterImpl::object_start(ObjectType type, IIndirectObject& obj)
{
    JAG_UNUSED_FUNCTION_ARGUMENT(type);
    JAG_UNUSED_FUNCTION_ARGUMENT(obj);

    if (m_pimpl->m_encryption_stream)
        m_pimpl->m_encryption_stream ->object_start(obj);
}


//////////////////////////////////////////////////////////////////////////
void DocWriterImpl::object_end(ObjectType type, IIndirectObject& obj)
{
    JAG_UNUSED_FUNCTION_ARGUMENT(type);
    JAG_UNUSED_FUNCTION_ARGUMENT(obj);

    if (m_pimpl->m_encryption_stream)
        m_pimpl->m_encryption_stream ->object_end(obj);
}


namespace {

const DocWriterImpl::FileID static_file_id = {
    0xc0, 0x8b, 0x63, 0xf2, 0xf5, 0x33, 0x27, 0x30, 0xfe, 0xf6, 0xd2, 0x9d, 0xc2, 0x2e, 0x85, 0x3a
};

} // namespace anonymous

//////////////////////////////////////////////////////////////////////////
DocWriterImpl::FileID const& DocWriterImpl::file_id()
{
    if (m_pimpl->m_static_file_id)
        return static_file_id;

    if (!m_pimpl->m_file_id[0])
    {
        jstd::MemoryStreamOutput mem_stream;
        ObjFmtBasic mem_writer(mem_stream, utf8_to_16be_stream());
        mem_writer.output(static_cast<Int>(m_pimpl->m_out_stream->tell()));
        mem_writer.output(DateWriter().date());
        mem_writer.output(m_pimpl->m_last_object_nr);
        jstd::md5_calculate(mem_stream.data(), static_cast<UInt>(mem_stream.tell()), m_pimpl->m_file_id);
    }

    return m_pimpl->m_file_id;
}


//////////////////////////////////////////////////////////////////////////
IExecContext const& DocWriterImpl::exec_context() const
{
    return m_pimpl->m_exec_context;
}

//////////////////////////////////////////////////////////////////////////
ResourceManagement& DocWriterImpl::res_mgm()
{
    return *m_pimpl->m_resource_management;
}

//////////////////////////////////////////////////////////////////////////
ResourceManagement const& DocWriterImpl::res_mgm() const
{
    return *m_pimpl->m_resource_management;
}

//////////////////////////////////////////////////////////////////////////
IResourceCtx& DocWriterImpl::resource_ctx()
{
    return *m_pimpl->m_resctx;
}

//////////////////////////////////////////////////////////////////////////
IResourceCtx const& DocWriterImpl::resource_ctx() const
{
    return *m_pimpl->m_resctx;
}



//////////////////////////////////////////////////////////////////////////
std::auto_ptr<CanvasImpl> DocWriterImpl::create_canvas_impl()
{
    return std::auto_ptr<CanvasImpl>(
        new CanvasImpl(*this,
                       m_pimpl->m_stream_filters,
                       m_pimpl->m_num_stream_filters));
}

//////////////////////////////////////////////////////////////////////////
std::auto_ptr<ContentStream> DocWriterImpl::create_content_stream()
{
    return create_content_stream(
        m_pimpl->m_stream_filters, m_pimpl->m_num_stream_filters);
}



std::auto_ptr<ContentStream>
DocWriterImpl::create_content_stream(StreamFilter const* filters, int num_filters)
{
    return std::auto_ptr<ContentStream>(
        new ContentStream(*this, filters, num_filters));
}


//////////////////////////////////////////////////////////////////////////
Int DocWriterImpl::version() const
{
    return m_pimpl->m_version;
}


/**
 * @brief Checks the document version.
 *
 * If the document is in the strict mode then an exception is thrown when
 * the requested version is greater then the actual version.
 *
 * @param version   requested version
 * @param feature   name of the feature (sent to the possible error message)
 * @param always_strict false - consult the configuration whether we are in strict mode
 *                      true  - sets strict mode unconditionally
 */
void DocWriterImpl::ensure_version(Int version, Char const* feature, bool always_strict) const
{
    if (version > m_pimpl->m_version)
    {
        int strict = 1;
        if (!always_strict)
            strict = exec_context().config().get_int("doc.strict_mode");

        if (strict)
        {
            throw exception_invalid_operation(msg_feature_not_supported(
                                                   feature ,
                                                   m_pimpl->m_version ,
                                                   version))
                                               << JAGLOC;
        }
    }
}


/**
 * @brief Retrieves number of the currently opened or the next page.
 *
 * @return zero-based page number
 */
Int DocWriterImpl::page_number() const
{
     return static_cast<int>(m_pimpl->m_catalog->num_pages());
}


IndirectObjectRef DocWriterImpl::page_ref(int page_num) const
{
    return m_pimpl->m_catalog->page_ref(page_num);
}


/**
 * @brief Provides stream converter from utf8 to utf16be
 *
 * The primary purpose is to have it instantiated only once and share
 * it among mulitple ObjFmtBasic objects.
 *
 * The retrieved reference should be used only in the current scope.
 *
 * @return stream converter
 */
UnicodeConverterStream& DocWriterImpl::utf8_to_16be_stream()
{
    return m_pimpl->m_utf8_to_16be;
}


/// zero-based
Double DocWriterImpl::page_height(int page_number) const
{
    const int num_done_pages(static_cast<int>(m_pimpl->m_catalog->num_pages()));
    JAG_PRECONDITION(page_number <= num_done_pages);

    if (page_number==num_done_pages)
    {
        // currently opened page
        JAG_ASSERT(m_pimpl->m_current_page.get());
        return m_pimpl->m_current_page->dimension()[1];
    }
    else
    {
        return m_pimpl->m_catalog->pages()[page_number].dimension()[1];
    }
}


/**
 * @brief Indicates whether a page is opened.
 *
 * Opened page means that we are somewhere between page_start() and
 * page_end().
 *
 * @return true if a page is opened, false otherwise.
 */
bool DocWriterImpl::is_page_opened() const
{
    return m_pimpl->m_current_page.get();
}

Destination DocWriterImpl::destination_reserve()
{
    std::auto_ptr<IndirectObjectFromDirect> dest(new IndirectObjectFromDirect(*this));
    m_pimpl->m_destinations.push_back(dest.release());
    return static_cast<Destination>(m_pimpl->m_destinations.size()) - 1;
}


void DocWriterImpl::destination_define_reserved(Destination id, Char const* dest)
{
    if (id>=m_pimpl->m_destinations.size() || m_pimpl->m_destinations[id].object())
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    std::auto_ptr<DestinationDef> dest_ptr(new DestinationDef(*this, dest));
    dest_ptr->set_page_num(page_number(), false);
    m_pimpl->m_destinations[id].object(std::auto_ptr<IDirectObject>(dest_ptr));
}


Destination DocWriterImpl::destination_define(Char const* dest)
{
    std::auto_ptr<DestinationDef> dest_ptr(new DestinationDef(*this, dest));
    dest_ptr->set_page_num(page_number(), false);

    std::auto_ptr<IndirectObjectFromDirect> adapter(
        new IndirectObjectFromDirect(*this, std::auto_ptr<IDirectObject>(dest_ptr)));

    m_pimpl->m_destinations.push_back(adapter.release());
    return static_cast<Destination>(m_pimpl->m_destinations.size()) - 1;
}


IndirectObjectRef DocWriterImpl::destination_ref(Destination id) const
{
    if (id>=m_pimpl->m_destinations.size())
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    return IndirectObjectRef(m_pimpl->m_destinations[id]);
}




///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// RESOURCE REGISTRATION/DEFINITION
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


//
//
//
Pattern DocWriterImpl::tiling_pattern_load(Char const* pattern, ICanvas* canvas)
{
    ensure_version(2, "tiling pattern");
    return id_from_handle<Pattern>(
        res_mgm().tiling_pattern_load(pattern, canvas));
}

//
//
//
Pattern DocWriterImpl::shading_pattern_load(Char const* pattern,
                                            ColorSpace cs,
                                            Function func)
{
    ensure_version(3, "shading pattern");
    FunctionHandle fn_handle(handle_from_id<RESOURCE_FUNCTION>(func));

    return id_from_handle<Pattern>(
        res_mgm().shading_pattern_load(
            pattern,
            handle_from_id<RESOURCE_COLOR_SPACE>(cs),
            &fn_handle, 1));
}

//
//
//
Pattern DocWriterImpl::shading_pattern_load_n(Char const* pattern,
                                              ColorSpace cs,
                                              Function const* array_in,
                                              UInt length)
{
    ensure_version(3, "shading pattern");
    if (!length)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    std::vector<FunctionHandle> functions(length);
    for(UInt i=0; i<length; ++i)
        functions[i] = handle_from_id<RESOURCE_FUNCTION>(array_in[i]);

    return id_from_handle<Pattern>(
        res_mgm().shading_pattern_load(
            pattern,
            handle_from_id<RESOURCE_COLOR_SPACE>(cs),
            &functions[0], length));
}

// ---------------------------------------------------------------------------
//                        Functions
//

//
//
//
Function DocWriterImpl::function_2_load(Char const* fun)
{
    ensure_version(3, "function type 2");
    return id_from_handle<Function>(
        res_mgm().function_2_load(fun));
}

//
//
//
Function DocWriterImpl::function_3_load(Char const* fun,
                                        Function const* array_in,
                                        UInt length)
{
    ensure_version(3, "function type 3");
    if (length < 2)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    std::vector<FunctionHandle> functions(length);
    for(UInt i=0; i<length; ++i)
        functions[i] = handle_from_id<RESOURCE_FUNCTION>(array_in[i]);

    return id_from_handle<Function>(
        res_mgm().function_3_load(fun, &functions[0], length));
}

//
//
//
Function DocWriterImpl::function_4_load(Char const* fun)
{
    ensure_version(3, "function type 4");
    return id_from_handle<Function>(
        res_mgm().function_4_load(fun));
}



///////////////////////////////////////////////////////////////////////////
// Fonts
//////////////////////////////////////////////////////////////////////////

// Loads a font from a fontspec.
//
// The font adapter is provided here instead of in FontManagement because:
//
// - IFontAdapter would have to implement IFontEx (currently only IFont)
// - PDFFont would reference IFontEx through the adapter
//
IFont* DocWriterImpl::font_load(Char const* fspec)
{
    PDFFont const& handle(res_mgm().fonts().font_load(fspec));
    IFontEx const* font = handle.font();

    DocWriterImpl_::FontMap::iterator it = m_pimpl->m_font_map.find(&handle);
    if (it != m_pimpl->m_font_map.end())
        return it->second.get();

    shared_ptr<IFontAdapter> obj;
    // Check whether a default text encoding is not specified. In such case we
    // need to install an adapter on IFont which translates between encodings.
    char const* txt_enc = get_default_text_encoding(exec_context());
    if (!is_empty(txt_enc))
    {
        char const* txt_enc_canon = get_canonical_converter_name(txt_enc);
        if (strcmp(font->encoding_canonical(), txt_enc_canon))
            obj.reset(new FontInfoReencoder(handle, txt_enc_canon));
    }

    if (!obj)
        obj.reset(new FontAdapter(handle));

    m_pimpl->m_font_map.insert(std::make_pair(&handle, obj));
    return obj.get();
}

///
/// Gives the default font for the document.
///
/// The font spec is take either from the fonts.default option or if
/// not provided then a hardcoded spec is used.
///
IFont* DocWriterImpl::default_font()
{
    if (!m_pimpl->m_default_font)
    {
        char const* font_spec = exec_context().config().get("fonts.default");
        JAG_ASSERT(!is_empty(font_spec));
        m_pimpl->m_default_font = font_load(font_spec);
    }

    return m_pimpl->m_default_font;
}


///////////////////////////////////////////////////////////////////////////
// Images, Masks
//////////////////////////////////////////////////////////////////////////

IImage* DocWriterImpl::image_load_file(Char const* image_file_path,
                                      ImageFormat image_type)
{
    return const_cast<IImageData*>(
        resource_ctx().image_man()->image_load_file(image_file_path,
                                                    image_type,
                                                    exec_context()));
}


IImage* DocWriterImpl::image_load(IImageDef* image)
{
    return const_cast<IImageData*>(
        resource_ctx().image_man()->image_load(image, exec_context()));
}


IImageDef* DocWriterImpl::image_definition() const
{
    return resource_ctx().image_man()->image_definition();
}





intrusive_ptr<IImageMask> DocWriterImpl::define_image_mask() const
{
    return resource_ctx().image_man()->define_image_mask();
}



ImageMaskID DocWriterImpl::register_image_mask(intrusive_ptr<IImageMask> image_mask)
{
    return id_from_handle<ImageMaskID>(
        resource_ctx().image_man()->register_image_mask(image_mask)
   );
}



///////////////////////////////////////////////////////////////////////////
// Color Spaces
//////////////////////////////////////////////////////////////////////////
ColorSpace DocWriterImpl::color_space_load(Char const* spec)
{
    IColorSpaceMan::cs_handle_pair_t handle(
        resource_ctx().color_space_man()->color_space_load(spec));

    JAG_ASSERT(is_valid(handle.first));

    if (CS_ICCBASED==color_space_type(handle.first))
        ensure_version(3, "ICC Based color space");

    if (CS_INDEXED==color_space_type(handle.first))
    {
        JAG_ASSERT(is_valid(handle.second));
        if (CS_ICCBASED==color_space_type(handle.second))
            ensure_version(3, "ICC Based color space");
    }

    return id_from_handle<ColorSpace>(handle.first);
}


//
//
//
ICanvas* DocWriterImpl::canvas_create() const
{
    return res_mgm().canvas_create();
}

//
//
// 
void DocWriterImpl::title(char const* title)
{
    IProfileInternal& profile = m_pimpl->m_exec_context.writable_config();
    profile.set("info.title", title);
}

//
//
// 
void DocWriterImpl::author(char const* author)
{
    IProfileInternal& profile = m_pimpl->m_exec_context.writable_config();
    profile.set("info.author", author);
}


//
//
// 
bool DocWriterImpl::is_topdown() const
{
    return m_pimpl->m_is_topdown;
}


void DocWriterImpl::add_output_intent(Char const* output_condition_id,
                                      Char const* iccpath,
                                      Char const* info,
                                      Int ncomponents,
                                      Char const* output_condition)
{
    ensure_version(4, "Output Intents");

    std::auto_ptr<output_intent_t> output_intent(new output_intent_t);

    output_intent->output_condition_id = output_condition_id;
    output_intent->ncomponents = ncomponents;

    if (!is_empty(iccpath))
        output_intent->icc_stream.reset(new FileStreamInput(iccpath));
    
    if (!is_empty(info))
        output_intent->info = info;
    
    if (!is_empty(output_condition))
        output_intent->output_condition = output_condition;
    
    m_pimpl->m_catalog->add_output_intent(output_intent);
}




}} //namespace jag::pdf



