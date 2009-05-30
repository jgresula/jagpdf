// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef IMAGEMASKIMPL_JG1636_H__
# define IMAGEMASKIMPL_JG1636_H__

#include <resources/interfaces/resourcehandle.h>
#include <resources/interfaces/imagemask.h>
#include <resources/interfaces/imagemaskdata.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace jag
{
// fwd
class IStreamInput;
namespace jstd { class MemoryStreamOutput; }

namespace resources
{

class ImageMaskImpl
    : public IImageMask
    , public IImageMaskData
{
public: //IImageMask
    void dimensions(UInt width, UInt height);
    void bit_depth(UInt bpc) { m_bpc = bpc; }
    void decode(Double lbound, Double ubound);
    void interpolate(Int val);
    void file_name(Char const* file_name);
    void matte(Double const* array, UInt length);
    void data(Byte const* array, UInt length);

public: //IImageMaskData
    bool output_mask(ISeqStreamOutput& fout, unsigned max_bits) const;
    UInt width() const { return m_width; }
    UInt height() const { return m_height; }
    InterpolateType interpolate() const { return m_interpolate; }
    UInt bits_per_component() const { return m_bpc; }
    ColorComponents const& matte() const { return m_matte; }
    DecodeArray const& decode() const { return m_decode; }

public:
    ImageMaskImpl();
    void check_consistency() const;

private: // Impl
    UInt                m_width;
    UInt                m_height;
    UInt                m_bpc;
    InterpolateType        m_interpolate;
    DecodeArray            m_decode;
    ColorComponents     m_matte;

    // data source variant
    mutable boost::shared_ptr<jstd::MemoryStreamOutput>    m_mask_data;
    std::string                                        m_file_name;
};


}} // namespace jag::resources

#endif // IMAGEMASKIMPL_JG1636_H__
/** EOF @file */
