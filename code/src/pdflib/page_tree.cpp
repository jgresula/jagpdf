// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "page_tree.h"
#include "page_tree_node.h"
#include "page_object.h"
#include <core/generic/refcountedimpl.h>
#include <boost/ref.hpp>
using namespace boost;

namespace jag {
namespace pdf {


PageTreeBuilder::PageTreeBuilder(DocWriterImpl& doc, int t)
    : m_doc(doc)
    , m_t(t)
    , m_max_children(2*t)
{
}


//
//
//
TreeNode::TreeNodePtr PageTreeBuilder::BuildPageTree(
    boost::ptr_vector<PageObject>& nodes)
{
    // copy page objects to buffer_a
    m_buffer_a.reserve(nodes.size());
    boost::ptr_vector<PageObject>::iterator it;
    for (it = nodes.begin(); it != nodes.end(); ++it)
    {
        TreeNode::TreeNodePtr data(&*it);
        m_buffer_a.push_back(data);
    }

    m_source = &m_buffer_a;

    // reserve some memory in the buffer_b
    m_buffer_b.reserve(nodes.size() / m_max_children + 1);
    m_dest = &m_buffer_b;

    // build tree levels, one at a time
    do
    {
        BuildOneTreeLevel();
        std::swap(m_source, m_dest);
        m_dest->resize(0);
    }
    while(m_source->size() > 1);

    if (m_source->size() == 1)
    {
        return (*m_source)[0];
    }
    else
    {
        return TreeNode::TreeNodePtr();
    }
}

//
//
//
void PageTreeBuilder::BuildOneTreeLevel()
{
    int remaining_nodes = static_cast<int>(m_source->size());
    int start_offset = 0;

    while(remaining_nodes)
    {
        int num_kids_for_this_node = 0;
        if (remaining_nodes >= m_max_children + m_t)
        {
            // at least two nodes are going to be created
            num_kids_for_this_node = m_max_children;
        }
        else if (remaining_nodes <= m_max_children)
        {
            // only single node is going to be created
            num_kids_for_this_node = remaining_nodes;
        }
        else
        {
            // exactly two nodes are going to be created, none of them is full
            num_kids_for_this_node = remaining_nodes / 2;
        }

        TreeNode::TreeNodePtr new_node = m_nodes.construct(ref(m_doc));
        m_dest->push_back(new_node);

        for (int i=start_offset; i<start_offset+num_kids_for_this_node; ++i)
        {
            new_node->add_kid(m_source->at(i));
        }

        start_offset += num_kids_for_this_node;
        remaining_nodes -= num_kids_for_this_node;
    }
}

}} //namespace jag::pdf
