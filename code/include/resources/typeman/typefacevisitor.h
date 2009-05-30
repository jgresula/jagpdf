// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TYPEFACEVISITOR_H_JAG_2336__
#define __TYPEFACEVISITOR_H_JAG_2336__

namespace jag
{

namespace resources
{
    class TypefaceTrueType;
}

class ITypefaceVisitor
{
public:
    virtual void visit(resources::TypefaceTrueType& obj) = 0;

protected:
    ~ITypefaceVisitor() {}
};

} // namespace jag

#endif //__TYPEFACEVISITOR_H_JAG_2336__
