// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEXOBJECT_H_JG2220__
#define __IMAGEXOBJECT_H_JG2220__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "indirectobjectref.h"
#include "indirectobjectfwd.h"
#include <resources/interfaces/resourcehandle.h>
#include <boost/scoped_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

namespace jag
{
class IImageData;

namespace pdf {

class DocWriterImpl;
class ContentStream;
class ImageXObjectSoftMask;
class ImageXObjectMask;
class ObjFmt;

class ImageXObject
    : public IndirectObjectFwd
{
public:
    DEFINE_VISITABLE;
    ImageXObject(DocWriterImpl& doc, IImageData const*img_data);

private: // IndirectObjectFwd
    void on_before_output();

private:
    void on_write(ObjFmt& fmt);
    void output_image_mask(ObjFmt& fmt);
    void output_rendering_intent(ObjFmt& fmt);
    void prepare_image_mask();

private:
    IImageData const*                   m_img_data;
    IndirectObjectRef                    m_img_mask_ref;
    IndirectObjectRef                     m_img_soft_mask_ref;
    IndirectObjectRef                    m_alternate_for_print;
    boost::scoped_ptr<ContentStream>    m_content_stream;
    DocWriterImpl&                        m_doc;
    IndirectObjectRef                   m_cs_ref;
    ColorSpaceHandle                    m_cs_handle;
    unsigned                            m_actual_bpc;
};

}} //namespace jag::pdf


#endif //__IMAGEXOBJECT_H_JG2220__

