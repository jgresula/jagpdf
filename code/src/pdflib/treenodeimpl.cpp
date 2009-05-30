// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "treenodeimpl.h"

namespace jag { namespace pdf
{

//////////////////////////////////////////////////////////////////////////
TreeNodeImpl::TreeNodeImpl(DocWriterImpl& doc)
    : TreeNode(doc)
    , m_leafs(0)
    , m_parent(0)
{
}

//////////////////////////////////////////////////////////////////////////
void TreeNodeImpl::add_kid(TreeNodePtr kid)
{
    m_kids.push_back(kid);

    if (!kid->num_kids())
        m_leafs += 1;

    m_leafs += kid->num_leafs();
    kid->set_parent(this);
}

//////////////////////////////////////////////////////////////////////////
int TreeNodeImpl::num_leafs() const
{
    return m_leafs;
}

//////////////////////////////////////////////////////////////////////////
int TreeNodeImpl::num_kids() const
{
    return static_cast<int>(m_kids.size());
}

//////////////////////////////////////////////////////////////////////////
TreeNode::TreeNodePtr& TreeNodeImpl::kid_at(int i)
{
    return m_kids[i];
}

//////////////////////////////////////////////////////////////////////////
void TreeNodeImpl::set_parent(TreeNode* parent)
{
    m_parent = parent;
}

//////////////////////////////////////////////////////////////////////////
TreeNode::TreeNodePtr const* TreeNodeImpl::kids_begin() const
{
    return m_kids.empty() ? NULL : &m_kids[0];
}

//////////////////////////////////////////////////////////////////////////
TreeNode::TreeNodePtr const* TreeNodeImpl::kids_end() const
{

    return kids_begin() + m_kids.size();
}

//////////////////////////////////////////////////////////////////////////
TreeNode::TreeNodePtr* TreeNodeImpl::kids_begin()
{
    return m_kids.empty() ? NULL : &m_kids[0];
}

//////////////////////////////////////////////////////////////////////////
TreeNode::TreeNodePtr* TreeNodeImpl::kids_end()
{
    return kids_begin() + m_kids.size();
}

}} //namespace jag::pdf
