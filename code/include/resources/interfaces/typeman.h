// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TYPEMAN_H_JAG_0008__
#define __TYPEMAN_H_JAG_0008__

#include <resources/interfaces/resourcehandle.h>
#include <resources/interfaces/fontspec.h>
#include <core/generic/noncopyable.h>
#include <boost/intrusive_ptr.hpp>

namespace jag {
class ITypefaceEx;
class ITypefaceSpec;
class IFontSpec;
class IFontEx;
class IExecContext;

class ITypeMan
    : public noncopyable
{
public:
    virtual boost::intrusive_ptr<IFontSpec> define_font() const = 0;
    virtual IFontEx const& font_load_spec(
        boost::intrusive_ptr<IFontSpec> const& font,
        IExecContext const&) = 0;

    virtual IFontEx const& font_load(char const* spec, IExecContext const&) = 0;


protected:
    ~ITypeMan() {}
};

} //namespace jag


#endif //__TYPEMAN_H_JAG_0008__
