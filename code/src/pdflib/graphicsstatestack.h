// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __GRAPHICSSTATESTACK_H_JAG_1532__
#define __GRAPHICSSTATESTACK_H_JAG_1532__

#include "graphicsstate.h"
#include <core/generic/noncopyable.h>
#include <resources/interfaces/resourcehandle.h>
#include <deque>

namespace jag
{
class IResourceCtx;

namespace pdf
{

class ObjFmtBasic;
class ResourceManagement;

///graphics state stack
class GraphicsStateStack
    : public noncopyable
{
public:
    GraphicsStateStack(DocWriterImpl& doc, ObjFmtBasic& fmt);
    GraphicsStateHandle save();
    void restore();
    GraphicsStateHandle commit();
    bool is_empty();
    GraphicsState& top();
    GraphicsState const& top() const;

private:
    DocWriterImpl&              m_doc;
    std::deque<GraphicsState>   m_stack;
    ObjFmtBasic&                m_fmt;
    GraphicsState               m_last_commited;
};


}} //namespace jag::pdf

#endif //__GRAPHICSSTATESTACK_H_JAG_1532__
