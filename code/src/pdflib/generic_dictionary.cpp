// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "generic_dictionary.h"
#include "docwriterimpl.h"
#include "objfmt.h"

namespace jag {
namespace pdf {

GenericDictionary::GenericDictionary(DocWriterImpl& doc)
    : IndirectObjectImpl(doc)
{
}

void GenericDictionary::on_output_definition()
{
    ObjFmt& writer = object_writer();
    writer.dict_start();
    m_dict_impl.output(writer);
    writer.dict_end();
}

void GenericDictionary::insert_string(const char* key, const char* value)
{
    m_dict_impl.insert_string(key, value);
}

void GenericDictionary::insert_bool(const char* key, bool value)
{
    m_dict_impl.insert_bool(key, value);
}


bool GenericDictionary::remove_key(const char* key)
{
    return m_dict_impl.remove_key(key);
}

bool GenericDictionary::empty() const
{
    return m_dict_impl.empty();
}

}} //namespace jag::pdf
