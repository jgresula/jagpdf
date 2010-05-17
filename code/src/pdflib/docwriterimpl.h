// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __DOCWRITERIMPL_H_JG1905__
#define __DOCWRITERIMPL_H_JG1905__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "interfaces/object_type.h"
#include "interfaces/objfmt_report.h"
#include "defines.h"
#include <pdflib/interfaces/pdfwriter.h>
#include <interfaces/streams.h>

#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

namespace jag {
// fwd
class IMessageSink;
class IProfileInternal;
class IExecContext;
class IResourceCtx;
class ICanvas;
class IImageDef;
class IImage;
class IImageMaskSpec;
class IImageSoftMaskSpec;
class IFontSpec;
class IFont;
class ISeqStreamOutputControl;

namespace jstd { class UnicodeConverterStream; }

namespace pdf
{
// fwd
class ObjFmt;
class IIndirectObject;
class IndirectObjectRef;
class CanvasImpl;
class ResourceManagement;
class ContentStream;
class FontManagement;


class DocWriterImplBaseInit
{
public:
    DocWriterImplBaseInit(IProfileInternal&);
    ~DocWriterImplBaseInit();
};


class DocWriterImpl
    : public DocWriterImplBaseInit
    , public IDocument
    , public IObjFmtReport
{
public: //IDocument
    void page_start(Double Width, Double Height);
    void page_end();
    void finalize();
    IPage* page();
    IDocumentOutline* outline();
    Int page_number() const;

    Destination destination_reserve();
    void destination_define_reserved(Destination id, Char const* dest);
    Destination destination_define(Char const* dest);

    IFont* font_load(Char const* fspec);

    ColorSpace color_space_load(Char const* spec);
    Pattern tiling_pattern_load(Char const* pattern, ICanvas* canvas);
    Pattern shading_pattern_load(Char const* pattern, ColorSpace cs, Function func);
    Pattern shading_pattern_load_n(Char const* pattern, ColorSpace cs,
                                   Function const* array_in, UInt length);

    Function function_2_load(Char const* fun);
    Function function_3_load(Char const* fun,
                             Function const* array_in,
                             UInt length);
    Function function_4_load(Char const* fun);

    IImageDef* image_definition() const;
    IImage* image_load(IImageDef* image);
    IImage* image_load_file(Char const* image_file_path, ImageFormat image_type);
    boost::intrusive_ptr<IImageMask> define_image_mask() const;
    ImageMaskID register_image_mask(boost::intrusive_ptr<IImageMask> image_mask);
    Int version() const;
    ICanvas* canvas_create() const;
    void title(char const* title);
    void author(char const* author);
    void add_output_intent(Char const* output_condition_id,
                           Char const* iccpath,
                           Char const* info,
                           Int ncomponents,
                           Char const* output_condition);
    
public:
    DocWriterImpl(
        boost::shared_ptr<ISeqStreamOutputControl> out_stream
        , boost::intrusive_ptr<IProfileInternal> config
   );

    ~DocWriterImpl();


public: //IObjFmtReport
    void object_start(ObjectType type, IIndirectObject& obj);
    void object_end(ObjectType type, IIndirectObject& obj);

public: //non-interface
    typedef Byte FileID[16];

    IMessageSink& message_sink();
    UInt assign_next_object_number();
    void add_indirect_object(Int object_number, Int generation_number, Int stream_offset);
    ObjFmt& object_writer() const;
    FileID const& file_id();
    void ensure_version(Int version, Char const* feature, bool always_strict=true) const;
    IExecContext const& exec_context() const;
    Double page_height(int page_number) const;
    bool is_page_opened() const;
    IndirectObjectRef destination_ref(Destination id) const;
    void output_destinations();

    ResourceManagement& res_mgm();
    ResourceManagement const& res_mgm() const;
    IResourceCtx& resource_ctx();
    IResourceCtx const& resource_ctx() const;
    IIndirectObject* doc_outline_if_nonempty() const;
    IndirectObjectRef page_ref(int page_num) const;
    jstd::UnicodeConverterStream& utf8_to_16be_stream();
    IFont* default_font();
    bool is_topdown() const;

    std::auto_ptr<CanvasImpl> create_canvas_impl();
    std::auto_ptr<ContentStream> create_content_stream();
    std::auto_ptr<ContentStream> create_content_stream(jag::pdf::StreamFilter const* filters, int num_filters);

private:
    struct DocWriterImpl_;
    boost::shared_ptr<DocWriterImpl_> m_pimpl;
};

}} //namespace jag::pdf


#endif //__DOCWRITERIMPL_H_JG1905__

