// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PAGE_TREE_NODE_H__2721837
#define __PAGE_TREE_NODE_H__2721837

#include "indirectobjectimpl.h"
#include "treenodeimpl.h"


namespace jag { namespace pdf
{

//fwd
class DocWriterImpl;

/// node of page tree
class PageTreeNode
    : public TreeNodeImpl
{
public:
    DEFINE_VISITABLE
    explicit PageTreeNode(DocWriterImpl& doc);

private: // IndirectObjectImpl
    void on_output_definition();
};

}} //namespace jag::pdf

#endif // __PAGE_TREE_NODE_H__2721837
