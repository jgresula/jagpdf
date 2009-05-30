// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TREE_NODE_H__132114
#define __TREE_NODE_H__132114

#include "indirectobjectimpl.h"
#include <boost/intrusive_ptr.hpp>

namespace jag { namespace pdf
{

/// base class for tree-like structures
class TreeNode
    : public IndirectObjectImpl
{
public:
    typedef TreeNode* TreeNodePtr;
    explicit TreeNode(DocWriterImpl& body)
        : IndirectObjectImpl(body)
    {}

    virtual void add_kid(TreeNodePtr kid) = 0;
    virtual int num_leafs() const = 0;
    virtual int num_kids() const = 0;
    virtual TreeNodePtr& kid_at(int i) = 0;
    virtual TreeNodePtr const* kids_begin() const = 0;
    virtual TreeNodePtr const* kids_end() const = 0;
    virtual TreeNodePtr* kids_begin() = 0;
    virtual TreeNodePtr* kids_end() = 0;
    virtual TreeNode* parent() = 0;
    virtual void set_parent(TreeNode* parent) = 0;
};

}} //namespace jag::pdf

#endif //__TREE_NODE_H__132114

