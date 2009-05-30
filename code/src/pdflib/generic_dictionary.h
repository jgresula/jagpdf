// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#ifndef __GENERIC_DICTIONARY_H__
#define __GENERIC_DICTIONARY_H__

#include "indirectobjectimpl.h"
#include "generic_dictionary_impl.h"
#include <core/jstd/memory_stream.h>

namespace jag { namespace pdf
{


class GenericDictionary
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE;
    explicit GenericDictionary(DocWriterImpl& doc);

    bool empty() const;
    void insert_string(const char* key, const char* value);
    void insert_bool(const char* key, bool value);
    bool remove_key(const char* key);

protected:
    void on_output_definition();

private:
    GenericDictionaryImpl    m_dict_impl;
};

}} //namespace jag::pdf

#endif //__GENERIC_DICTIONARY_H__
