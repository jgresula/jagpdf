// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "imagemaskimpl.h"
#include "imagemanip.h"
#include <msg_resources.h>
#include <core/generic/checked_cast.h>
#include <core/jstd/streamhelpers.h>
#include <core/jstd/memory_stream.h>
#include <core/jstd/file_stream.h>
#include <boost/scoped_ptr.hpp>

using namespace boost;

namespace jag {
namespace resources {

//////////////////////////////////////////////////////////////////////////
ImageMaskImpl::ImageMaskImpl()
    : m_width(0)
    , m_height(0)
    , m_bpc(0)
    , m_interpolate(INTERPOLATE_UNDEFINED)
{
}

//////////////////////////////////////////////////////////////////////////
void ImageMaskImpl::check_consistency() const
{
    if (!m_width || !m_height)
        throw exception_invalid_value(msg_image_mask_dimension_not_defined()) << JAGLOC;

    if (!m_bpc)
        throw exception_invalid_value(msg_image_mask_bpc_not_defined()) << JAGLOC;

    if (m_file_name.empty() && !m_mask_data)
        throw exception_invalid_value(msg_image_mask_data_not_specified()) << JAGLOC;
}


//////////////////////////////////////////////////////////////////////////
void ImageMaskImpl::dimensions(UInt width, UInt height)
{
    m_width = width;
    m_height = height;
}

//////////////////////////////////////////////////////////////////////////
void ImageMaskImpl::interpolate(Int val)
{
    m_interpolate = val ? INTERPOLATE_YES : INTERPOLATE_NO ;
}

//////////////////////////////////////////////////////////////////////////
void ImageMaskImpl::decode(Double lbound, Double ubound)
{
    Double arr[2] = { lbound, ubound };
    m_decode.reset(arr, 2);
}

//////////////////////////////////////////////////////////////////////////
void ImageMaskImpl::matte(Double const* array, UInt length)
{
    m_matte.reset(array, length);
}

//////////////////////////////////////////////////////////////////////////
void ImageMaskImpl::file_name(Char const* file_name)
{
    m_file_name = file_name;
    m_mask_data.reset();
}

//////////////////////////////////////////////////////////////////////////
void ImageMaskImpl::data(Byte const* data, UInt length)
{
    if (!m_mask_data)
        m_mask_data.reset(new jstd::MemoryStreamOutput);

    m_mask_data->write(data, length);
    std::string().swap(m_file_name);
}


//////////////////////////////////////////////////////////////////////////
bool ImageMaskImpl::output_mask(ISeqStreamOutput& dest, unsigned max_bits) const
{
    // prepare stream with data
    shared_ptr<IStreamInput> in_stream;
    if (!m_file_name.empty())
    {
        in_stream.reset(new jstd::FileStreamInput(m_file_name.c_str()));
    }
    else
    {
        in_stream.reset(new jstd::MemoryStreamInputFromOutput(m_mask_data));
    }


    if (m_bpc > max_bits)
    {
        JAG_PRECONDITION(m_bpc==16 && max_bits==8);
        downsample_big16_to_8(*in_stream, dest);
    }
    else
    {
        jstd::copy_stream(*in_stream, dest);
    }
    m_mask_data.reset();
    return true;
}


}} // namespace jag::resources

/** EOF @file */
