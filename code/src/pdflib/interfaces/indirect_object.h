// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __INDIRECT_OBJECT_H__1422049
#define __INDIRECT_OBJECT_H__1422049

#include "pdfobject.h"
#include <interfaces/stdtypes.h>
#include <cstring>

#include <limits>

namespace jag {
namespace pdf {

/// basic pdf object interface
class IIndirectObject
    : public IPdfObject
{
public:
    /**
     * Outputs the object definition.
     *
     * No stream parameter is passed. The object knows where to
     * output the definition.
     */
    virtual void output_definition() = 0;

    /**
     * @return object number
     */
    virtual Int object_number() const = 0;

    /**
     * @return generation number
     */
    virtual Int generation_number() const = 0;

    /// dtor
    virtual ~IIndirectObject() {};
};

const std::size_t INDIRECT_OBJECT_REFERENCE_MAX_TEXT_SIZE = 2*(std::numeric_limits<UInt>::digits10 + 3) + 5;
int make_ref(Int object_nr, Int generation_nr, char* buffer);

}} //namespace jag::pdf

#endif //__INDIRECT_OBJECT_H__1422049

