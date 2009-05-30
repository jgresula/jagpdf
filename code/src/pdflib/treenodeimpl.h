// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TREE_NODE_IMPL_H__932130
#define __TREE_NODE_IMPL_H__932130

#include "indirectobjectimpl.h"
#include "tree_node.h"
#include <boost/enable_shared_from_this.hpp>
#include <vector>

namespace jag { namespace pdf
{

/// default implementation of TreeNode
class TreeNodeImpl
    : public TreeNode
{
public: //ITreeNode
    void add_kid(TreeNodePtr kid);
    int num_leafs() const;
    int num_kids() const;
    TreeNodePtr& kid_at(int i);
    TreeNodePtr const* kids_begin() const;
    TreeNodePtr const* kids_end() const;
    TreeNodePtr* kids_begin();
    TreeNodePtr* kids_end();
    TreeNode* parent() { return m_parent; }
    void set_parent(TreeNode* parent);

public:
    explicit TreeNodeImpl(DocWriterImpl& body);

protected:
    ~TreeNodeImpl() {};


private:
    std::vector<TreeNodePtr>    m_kids;
    int                         m_leafs;
    TreeNode*                   m_parent;
};

}} //namespace jag::pdf

#endif //__TREE_NODE_IMPL_H__932130
