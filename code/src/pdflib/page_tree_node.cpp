// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "page_tree_node.h"
#include "docwriterimpl.h"
#include "objfmt.h"

namespace jag { namespace pdf
{


PageTreeNode::PageTreeNode(DocWriterImpl& doc)
    : TreeNodeImpl(doc)
{
};


void  PageTreeNode::on_output_definition()
{
    ObjFmt& writer = object_writer();

    writer.dict_start()
        .dict_key("Type").output("Pages")
        .dict_key("Count").space().output(num_leafs())
    ;

    if (parent())
        writer.dict_key("Parent").space().ref(*parent());

    writer.dict_key("Kids");
    writer.array_start();
    for (int i=0; i<num_kids(); ++i)
    {
        if (i != 0)
            writer.space();

        writer.ref(*kid_at(i));
    }
    writer.array_end()
        .dict_end()
    ;
}

}} //namespace jag::pdf
