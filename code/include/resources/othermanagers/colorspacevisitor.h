// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __COLORSPACEVISITOR_JG1035_H__
#define __COLORSPACEVISITOR_JG1035_H__

#include <core/generic/noncopyable.h>

namespace jag {

//fwd
namespace pdf
{
  class PatternColorSpace;
}

//fwd
namespace resources
{
  class PaletteImpl;
  class CIELabImpl;
  class CIECalRGBImpl;
  class CIECalGrayImpl;
  class ICCBasedImpl;
}

class IColorSpaceVisitor
    : public noncopyable
{
public:
    virtual void visit(resources::PaletteImpl& obj) = 0;
    virtual void visit(resources::CIELabImpl& obj) = 0;
    virtual void visit(resources::CIECalRGBImpl& obj) = 0;
    virtual void visit(resources::CIECalGrayImpl& obj) = 0;
    virtual void visit(pdf::PatternColorSpace& obj) = 0;
    virtual void visit(resources::ICCBasedImpl& obj) = 0;

protected:
    ~IColorSpaceVisitor() {}
};

#define JAG_VISITABLE_COLOR_SPACE\
    void accept(IColorSpaceVisitor& v) { v.visit(*this); }


} // namespace jag::resources

#endif // __COLORSPACEVISITOR_JG1035_H__
/** EOF @file */
