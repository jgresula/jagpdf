// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "generic_dictionary_impl.h"
#include "objfmt.h"
#include "interfaces/indirect_object.h"

namespace jag {
namespace pdf{

//////////////////////////////////////////////////////////////////////////
class OutputVisitor
    : public boost::static_visitor<>
{
public:
    OutputVisitor(ObjFmt& writer)
        : m_writer(writer)
    {}

    void operator()(GenericDictionaryImpl::String const& str) {
        m_writer.text_string(str.m_data.c_str());
    }

    void operator()(GenericDictionaryImpl::RawString const& str) {
        m_writer.raw_text(str.m_data.c_str());
    }

    void operator()(unsigned int const& value) {
        m_writer.output(value);
    }

    void operator()(bool const& value) {
        m_writer.output_bool(value);
    }

    void operator()(boost::shared_ptr<IIndirectObject> const& obj) {
        m_writer.ref(*obj);
    }

private:
    ObjFmt& m_writer;
};


//////////////////////////////////////////////////////////////////////////
void GenericDictionaryImpl::insert_string(const char* key, const char* value)
{
    m_map.insert(std::make_pair(key, String(string_32_cow(value))));
}


//////////////////////////////////////////////////////////////////////////
bool GenericDictionaryImpl::remove_key(const char* key)
{
    Map::iterator it = m_map.find(key);
    if (it != m_map.end())
    {
        m_map.erase(it);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
void GenericDictionaryImpl::output(ObjFmt& writer)
{
    OutputVisitor variant_visitor(writer);
    for(Map::iterator it = m_map.begin(); it != m_map.end(); ++it)
    {
        writer.dict_key(it->first).space();
        boost::apply_visitor(variant_visitor, it->second);
    }
}

////////////////////////////////////////////////////////////////////////
void GenericDictionaryImpl::insert_key(const char* key, boost::shared_ptr<IIndirectObject> const& obj)
{
    m_map.insert(std::make_pair(key, obj));
}

//////////////////////////////////////////////////////////////////////////
void GenericDictionaryImpl::insert_key(const char* key, unsigned int value)
{
    m_map.insert(std::make_pair(key, value));
}

//
//
//
void GenericDictionaryImpl::insert_bool(const char* key, bool value)
{
    m_map.insert(std::make_pair(key, value));
}


}} //namespace jag::pdf
