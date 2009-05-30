// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __INDIRECTOBJECTCALLBACK_JG1355_H__
#define __INDIRECTOBJECTCALLBACK_JG1355_H__

#include "indirectobjectimpl.h"
#include <boost/function.hpp>

namespace jag {
namespace pdf {

/**
 * @brief IIndirect object implementation with callbacks.
 */
class IndirectObjectCallback
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE;
    typedef boost::function<void (ObjFmt&)> output_callback_t;
    typedef boost::function<bool (ObjFmt&)> before_callback_t;

    explicit IndirectObjectCallback(DocWriterImpl& doc, output_callback_t const& on_output);
    IndirectObjectCallback(DocWriterImpl& doc, output_callback_t const& on_output, before_callback_t const& on_before_output);

private: //IndirectObjectImpl
    void on_output_definition();
    bool on_before_output_definition();

private:
    output_callback_t  m_on_output;
    before_callback_t  m_on_before_output;
};


}} // namespace jag::pdf

#endif // __INDIRECTOBJECTCALLBACK_JG1355_H__
/** EOF @file */
