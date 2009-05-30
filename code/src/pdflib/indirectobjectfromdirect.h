// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef INDIRECTOBJECTFROMDIRECT_JG1723_H__
#define INDIRECTOBJECTFROMDIRECT_JG1723_H__

#include "indirectobjectimpl.h"
#include <boost/shared_ptr.hpp>

namespace jag {
namespace pdf {

class DocWriterImpl;
class IDirectObject;

class IndirectObjectFromDirect
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE
    explicit IndirectObjectFromDirect(DocWriterImpl& doc);
    IndirectObjectFromDirect(DocWriterImpl& doc, std::auto_ptr<IDirectObject> obj);
    IndirectObjectFromDirect(DocWriterImpl& doc, IDirectObject& obj);
    IDirectObject* object() const;
    void object(std::auto_ptr<IDirectObject>);

private:
    void on_output_definition();

private:
    boost::shared_ptr<IDirectObject> m_direct;
};

}} // namespace jag:pdf

#endif // INDIRECTOBJECTFROMDIRECT_JG1723_H__
/** EOF @file */
