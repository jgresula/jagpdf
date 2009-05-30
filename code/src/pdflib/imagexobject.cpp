// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "imagexobject.h"

#include "objfmt.h"
#include "docwriterimpl.h"
#include "contentstream.h"
#include "colorspace.h"
#include "resourcemanagement.h"
#include "defines.h"
#include "xobjectutils.h"
#include <msg_pdflib.h>
#include <resources/resourcebox/resourcepodarrays.h>
#include <resources/interfaces/resourcectx.h>
#include <resources/interfaces/colorspaceman.h>
#include <resources/interfaces/imageman.h>
#include <resources/interfaces/imagemaskdata.h>

#include <core/errlib/msg_writer.h>
#include <resources/interfaces/imagedata.h>
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>
#include <boost/bind.hpp>

using namespace jag::resources;
using namespace boost;

namespace jag {
namespace pdf {


//////////////////////////////////////////////////////////////////////////
ImageXObject::ImageXObject(DocWriterImpl& doc, IImageData const* img_data)
    : m_img_data(img_data)
    , m_doc(doc)
    , m_actual_bpc(0)
{
    // setup stream filter according to image type
    switch(m_img_data->format())
    {
    case IMAGE_FORMAT_JPEG:
        {
            const StreamFilter filter = STREAM_FILTER_DCTDECODE;
            m_content_stream.reset(m_doc.create_content_stream(&filter, 1).release());
        }
        break;

    default:
        // use default stream settings
        m_content_stream.reset(m_doc.create_content_stream().release());
    }

    JAG_ASSERT(m_content_stream);

    m_content_stream->set_writer_callback(
        boost::bind(&ImageXObject::on_write, this, _1));

    reset_indirect_object_worker(m_content_stream.get());
}


//
//
//
void ImageXObject::prepare_image_mask()
{
    ImageMaskHandle mask_handle(m_img_data->image_mask());
    JAG_ASSERT(is_valid(mask_handle));  // an user violates precondition

    IImageMaskData const& mask_data(
        m_doc.resource_ctx().image_man()->image_mask_data(mask_handle));

    unsigned mask_type = detect_mask_type(mask_data, m_doc);
    if (DMT_SOFT == mask_type)
    {
        // -- soft mask
        unsigned err_code = 0;

        // check whether matte is not used - in that case
        // the mask and image dimensions must be the same
        if (is_valid(mask_data.matte()))
        {
            if (m_img_data->width() != mask_data.width()
                || m_img_data->height() != mask_data.height())
            {
                err_code = WRN_MATTE_DIMENSIONS_MISMATCH;
                write_message(WRN_MATTE_DIMENSIONS_MISMATCH);
            }
        }

        if (!err_code)
        {
            // ok, use the soft mask
            m_img_soft_mask_ref.reset(
                m_doc.res_mgm().image_mask_reference(mask_handle));
        }
    }
    else if (DMT_HARD == mask_type)
    {
        m_img_mask_ref.reset(
            m_doc.res_mgm().image_mask_reference(mask_handle));
    }
    else
    {
        write_message(WRN_IMAGE_MASK_IGNORED);
    }
}



//////////////////////////////////////////////////////////////////////////
void ImageXObject::on_before_output()
{
    // ensure that a mask represent by an indirect object is outputted
    // before this object; do so by taking a reference to the mask
    if (m_img_data->image_mask_type() == IMT_IMAGE)
        prepare_image_mask();

     // alternate for printing
    ImageHandle alternate = m_img_data->alternate_for_printing();
    if (is_valid(alternate))
    {
        m_doc.ensure_version(3, "alternate for printing");
        m_alternate_for_print.reset(
            m_doc.res_mgm().image_reference(alternate));
    }


    // color space
    m_cs_handle = m_img_data->color_space();
    if (is_valid(m_cs_handle))
    {
        if (CS_ICCBASED==color_space_type(m_cs_handle)
             && m_doc.version() < 3)
        {
            // downgrade icc to device cs
            // in the future that can be done better by
            // image filters (e.g. icc -> calrgb)
            intrusive_ptr<IColorSpace> cs(m_doc.resource_ctx().color_space_man()->color_space(m_cs_handle));
            switch(cs->num_components())
            {
            case 1: m_cs_handle = ColorSpaceHandle(CS_DEVICE_GRAY); break;
            case 3: m_cs_handle = ColorSpaceHandle(CS_DEVICE_RGB); break;
            case 4: m_cs_handle = ColorSpaceHandle(CS_DEVICE_CMYK); break;

            default:
                // pdf specs defines that icc color space can have
                // 1, 3 or 4 channels
                JAG_INTERNAL_ERROR;
            }
        }
        else
        {
            if (!resources::is_trivial_color_space(m_cs_handle))
                m_cs_ref = m_doc.res_mgm().color_space_ref(m_cs_handle);
        }
    }


    // 16-bits supported since 1.5
    m_actual_bpc = m_img_data->bits_per_component()>8 && m_doc.version()<5
        ? 8
        : m_img_data->bits_per_component();

    // send image data to the content stream
    if (!m_img_data->output_image(m_content_stream->stream(), m_actual_bpc))
        throw exception_invalid_value(msg_16bits_since_15()) << JAGLOC;
}




//////////////////////////////////////////////////////////////////////////
void ImageXObject::on_write(ObjFmt& fmt)
{
    fmt
        .dict_key("Type").output("XObject")
        .dict_key("Subtype").output("Image")
        .dict_key("Width").space().output( m_img_data->width())
        .dict_key("Height").space().output(m_img_data->height())
        .dict_key("BitsPerComponent").space().output(m_actual_bpc)
    ;

    // color space
    if (is_valid(m_cs_ref))
    {
        // non-trivial color space
        fmt.dict_key("ColorSpace").space();
        fmt.ref(m_cs_ref);
    }
    else if (is_valid(m_cs_handle))
    {
        // trivial color space
        fmt.dict_key("ColorSpace");
        output_trivial_color_space_name(m_cs_handle, fmt.fmt_basic());
    }


    output_image_mask(fmt);
    xobject_output(fmt.fmt_basic(), m_img_data->interpolate(), m_doc.exec_context().config());
    xobject_output(fmt.fmt_basic(), m_img_data->decode());
    output_rendering_intent(fmt);

    if (is_valid(m_alternate_for_print))
    {
        fmt
            .dict_key("Alternates")
            .array_start()
            .dict_start()
            .dict_key("Image").space().ref(m_alternate_for_print)
            .dict_key("DefaultForPrinting").space().output_bool(true)
            .dict_end()
            .array_end()
        ;
    }

/*
- StructParent
- ID
- OPI
- Metadata
- OC
JPEG
 - mask

*/
}

//////////////////////////////////////////////////////////////////////////
void ImageXObject::output_rendering_intent(ObjFmt& fmt)
{
    RenderingIntentType rintent = m_img_data->rendering_intent();
    if (rintent == RI_UNDEFINED)
    {
        rintent = get_enum<RenderingIntentType>
            (   m_doc.exec_context().config(), "images.intent");
    }

    if (rintent != RI_UNDEFINED)
    {
        fmt
            .dict_key("Intent")
            .space()
            .output(rendering_intent_string(rintent))
        ;
    }
}



//////////////////////////////////////////////////////////////////////////
void ImageXObject::output_image_mask(ObjFmt& fmt)
{
    switch (m_img_data->image_mask_type())
    {
    case IMT_COLOR_KEY:
        if (m_doc.version() < 3)
        {
            write_message(WRN_COLOR_KEY_MASK_IGNORED);
        }
        else
        {
            ColorKeyMaskArray const& mask = m_img_data->color_key_mask();
            fmt.dict_key("Mask").array_start();
            for(size_t i=0; i<mask.size(); ++i)
                fmt.output(mask.data()[i]).space();
            fmt.array_end();
        }
        break;

    case IMT_IMAGE:
        if (is_valid(m_img_soft_mask_ref)) {
            fmt.dict_key("SMask").space().ref(m_img_soft_mask_ref);
        } else if (is_valid(m_img_mask_ref)) {
            fmt.dict_key("Mask").space().ref(m_img_mask_ref);
        }
        break;

    default:
        ;
    }
}

}} //namespace jag::pdf

