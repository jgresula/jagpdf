// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "imagexobjectmask.h"
#include "docwriterimpl.h"
#include "contentstream.h"
#include "objfmt.h"
#include "xobjectutils.h"
#include <msg_pdflib.h>

#include <core/errlib/msg_writer.h>
#include <core/generic/assert.h>
#include <resources/interfaces/imagemaskdata.h>
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>
#include <boost/bind.hpp>

namespace jag {
namespace pdf {

//////////////////////////////////////////////////////////////////////////
ImageXObjectMask::ImageXObjectMask(DocWriterImpl& doc, IImageMaskData const& img_mask)
    : m_img_mask(img_mask)
    , m_content_stream(doc.create_content_stream())
    , m_doc(doc)
    , m_actual_bpc(0)
    , m_mask_type(detect_mask_type(img_mask, doc))
{
    JAG_ASSERT(m_mask_type != DMT_NOT_SUPPORTED);

    m_content_stream->set_writer_callback(
        boost::bind(&ImageXObjectMask::on_write, this, _1));

    reset_indirect_object_worker(m_content_stream.get());
}


//////////////////////////////////////////////////////////////////////////
void ImageXObjectMask::on_before_output()
{
    if (m_mask_type == DMT_SOFT)
    {
        // 16-bits supported since 1.5
        int use_8 = m_doc.exec_context().config().get_int("images.softmask_16_to_8");
        m_actual_bpc = use_8 || m_doc.version()<5
            ? (std::min)(m_img_mask.bits_per_component(), 8u)
            : m_img_mask.bits_per_component();
        ;

        // send mask data to the content stream
        if (!m_img_mask.output_mask(m_content_stream->stream(), m_actual_bpc))
            throw exception_invalid_value(msg_16bits_since_15()) << JAGLOC;
    }
    else
    {
        m_img_mask.output_mask(m_content_stream->stream());
    }
}



//////////////////////////////////////////////////////////////////////////
void ImageXObjectMask::on_write(ObjFmt& fmt)
{
    if (m_mask_type == DMT_SOFT)
    {
        fmt
            .dict_key("Type").output("XObject")
            .dict_key("Subtype").output("Image")
            .dict_key("Width").space().output( m_img_mask.width())
            .dict_key("Height").space().output(m_img_mask.height())
            .dict_key("ColorSpace").output("DeviceGray")
            .dict_key("BitsPerComponent").space().output(m_actual_bpc)
            ;

        xobject_output(fmt.fmt_basic(), m_img_mask.decode());
        xobject_output(fmt.fmt_basic(), m_img_mask.interpolate(), m_doc.exec_context().config());

        // pre-blended image
        ColorComponents const& matte = m_img_mask.matte();
        if (is_valid(matte))
        {
            fmt    .dict_key("Matte").array_start();

            for (size_t i=0; i<matte.size(); ++i)
                fmt.output(matte.data()[i]).space();

            fmt.array_end();
        }
    }
    else
    {
        fmt
            .dict_key("Type").output("XObject")
            .dict_key("Subtype").output("Image")
            .dict_key("Width").space().output( m_img_mask.width())
            .dict_key("Height").space().output(m_img_mask.height())
            .dict_key("ImageMask").space().output_bool(true)
            // <quote from 'stencil masking'>
            // The Decode entry determines how the source samples are to be
            // interpreted. If the Decode array is [0 1] (the default for an
            // image mask), a sample value of 0 marks the page with the current
            // color, and a 1 leaves the previous contents unchanged. If the
            // Decode array is [1 0], these meanings are reversed.
            // </quote>
            //
            // We must use [1 0] as a soft mask has 0.0 outside the clipping
            // path and 1.0 inside.
            .dict_key("Decode").raw_bytes("[1 0]", 5);
            ;


        xobject_output(fmt.fmt_basic(), m_img_mask.interpolate(), m_doc.exec_context().config());
    }
}

}} //namespace jag::pdf

