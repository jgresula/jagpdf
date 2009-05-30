// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __GENERIC_DICTIONARY_IMPL_H__
#define __GENERIC_DICTIONARY_IMPL_H__

#include <core/generic/stringutils.h>
#include <external/flex_string/string.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/variant.hpp>
#include <map>

namespace jag { namespace pdf
{

class ObjFmt;
class IIndirectObject;

class GenericDictionaryImpl
    : public boost::noncopyable
{
public:
    bool empty() const { return m_map.empty(); }
    void insert_string(const char* key, const char* value);
    void insert_key(const char* key, boost::shared_ptr<IIndirectObject> const& obj);
    void insert_key(const char* key, unsigned int value);
    void insert_bool(const char* key, bool value);
    bool remove_key(const char* key);
    void output(ObjFmt& writer);

private:
    friend class OutputVisitor;
    template<class T, int I>
    struct UniqueType
    {
        UniqueType(T const&  t) : m_data(t) {}
        typedef T type;
        enum { ord_type = I };
        type m_data;
    };

    typedef UniqueType<string_32_cow,1> String;
    typedef UniqueType<string_32_cow,2> RawString;

    typedef boost::variant<
        bool
      , String
      , RawString
      , unsigned int
      , boost::shared_ptr<IIndirectObject>
    >    Variant;

    typedef std::map<char const*, Variant, string_compare> Map;
    Map m_map;
};

}} //namespace jag::pdf

#endif //__GENERIC_DICTIONARY_IMPL_H__
