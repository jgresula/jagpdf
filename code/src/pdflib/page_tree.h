// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PAGE_TREE_H__2721830
#define __PAGE_TREE_H__2721830

#include "tree_node.h"
#include "page_tree_node.h"
#include "interfaces/indirect_object.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/pool/object_pool.hpp>

namespace jag { namespace pdf
{

//fwd
class PageObject;
class DocWriterImpl;

class PageTreeBuilder
{
public:
    PageTreeBuilder(DocWriterImpl& body, int t);
    TreeNode::TreeNodePtr BuildPageTree(boost::ptr_vector<PageObject>& nodes);

private:
    void BuildOneTreeLevel();

private:
    DocWriterImpl&    m_doc;
    const int       m_t;
    const int       m_max_children;

    typedef std::vector<TreeNode::TreeNodePtr> NodeVector;

    NodeVector*   m_source;
    NodeVector*   m_dest;
    NodeVector    m_buffer_a;
    NodeVector    m_buffer_b;

    boost::object_pool<PageTreeNode>  m_nodes;
};

}} //namespace jag::pdf

#endif //__PAGE_TREE_H__2721830
