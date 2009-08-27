// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TYPEFACEUTILS_H_JAG_1133__
#define __TYPEFACEUTILS_H_JAG_1133__

#include <core/generic/floatpointtools.h>
#include <msg_resources.h>
#include <resources/interfaces/typeface.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace jag {
//fwd
class IStreamInput;

namespace resources {

#define CHECK_FT(err) if (err) {                                \
        throw jag::exception_operation_failed(msg_freetype_failed(err)) << JAGLOC; }


/**
 * @brief converts from font units to glyph units (1/72000)
 *
 * font unit is em_space*1/72 dpi, glyph unit is 1000*dpi
 * em-square is 1 unit, i.e 1/72 inch
 */
class FontSpaceToGlyphSpace
{
public:
    FontSpaceToGlyphSpace(TypefaceMetrics const& metrics)
        : m_coef(1000.0/metrics.units_per_EM)
    {}

    int operator()(int fs) const {
        return round(fs*m_coef);
    }

private:
    const double m_coef;
};


std::auto_ptr<IStreamInput>
create_ftopenargs_stream_adapter(FT_Open_Args const& args);




}} // namespace jag::resources

#endif //__TYPEFACEUTILS_H_JAG_1133__
