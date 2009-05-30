// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CONFIGIMPL_H__
#define __CONFIGIMPL_H__

#include <interfaces/configinternal.h>
#include <interfaces/stdtypes.h>
#include <external/flex_string/string.h>
#include <boost/intrusive_ptr.hpp>
#include <map>

namespace jag {
// fwd
class ISeqStreamOutput;

namespace jstd {

// fwd
class ConfigImpl;


/// single (config, value) pair
struct cfg_pair_t
{
    char const* name;
    char const* default_value;
};



/// Loads the configuration from a string
boost::intrusive_ptr<ConfigImpl>
load_from_string(Char const* str, cfg_pair_t const* symbols);


/// Loads the configuration from a file
boost::intrusive_ptr<ConfigImpl>
load_from_file(Char const* fname, cfg_pair_t const* symbols);





///
/// Configuration object.
///
class ConfigImpl
    : public IProfileInternal
{
public: // IProfile
    void set(Char const* kwd, Char const* value);
    void save_to_file(Char const* fname) const;

public: // IProfileInternal
    Char const* get(Char const* kwd) const;
    int get_int(Char const* kwd) const;
    boost::intrusive_ptr<IProfileInternal> clone() const;
    void verification_active(bool value);

public: // internal
    explicit ConfigImpl(cfg_pair_t const* symbols);
    void verify(Char const* kwd) const;
    void save(ISeqStreamOutput& stream) const;
    void set_r(Char const* kwd_begin, Char const* kwd_end,
                Char const* value_begin, Char const* value_end);

private:
    typedef string_32_cow string_t;
    typedef std::map<string_t,string_t> config_map_t;

    cfg_pair_t const* m_symbols;
    bool              m_verification_active;
    config_map_t      m_config_map;
};


//
//
//
inline void ConfigImpl::verification_active(bool value)
{
    m_verification_active = value;
}


}} //namespace jag::jstd

#endif //__CONFIGIMPL_H__
