// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEMASKDATA_H_JG_2001__
#define __IMAGEMASKDATA_H_JG_2001__

#include <resources/interfaces/resourceconstants.h>
#include <resources/resourcebox/resourcepodarrays.h>
#include <interfaces/refcounted.h>

namespace jag {
//fwd
class ISeqStreamOutput;

class IImageMaskData
    : public IRefCounted
{
public:
    virtual bool output_mask(ISeqStreamOutput& fout, unsigned max_bits=16) const = 0;
    virtual UInt width() const = 0;
    virtual UInt height() const = 0;
    virtual UInt bits_per_component() const = 0;
    virtual DecodeArray const& decode() const = 0;
    virtual InterpolateType interpolate() const = 0;
    virtual ColorComponents const& matte() const = 0;

protected:
    ~IImageMaskData() {}
};

} // namespace jag

#endif //__IMAGEMASKDATA_H_JG_2001__


