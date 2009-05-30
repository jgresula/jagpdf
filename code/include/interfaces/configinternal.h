// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef CONFIGINTERNAL_JG2051_H__
#define CONFIGINTERNAL_JG2051_H__

#include <interfaces/configuration.h>

namespace jag {

class IProfileInternal
    : public IProfile
{
    /**
     * @brief Clones the configuration object.
     *
     * @return A copy of the configuration object.
     */
public:
    virtual boost::intrusive_ptr<IProfileInternal> clone() const = 0;

    virtual Char const* get(Char const* option) const = 0;
    virtual int get_int(Char const* option) const = 0;
    virtual void verification_active(bool value) = 0;
};


template<class T>
T get_enum(IProfileInternal const& cfg, Char const* kwd)
{
    return static_cast<T>(cfg.get_int(kwd));
}



} // namespace jag

#endif // CONFIGINTERNAL_JG2051_H__
/** EOF @file */
