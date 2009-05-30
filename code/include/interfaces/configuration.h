// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CONFIGURATION_H_JG1642__
#define __CONFIGURATION_H_JG1642__

#include <interfaces/stdtypes.h>
#include <interfaces/refcounted.h>
#include <boost/intrusive_ptr.hpp>

namespace jag
{
/// Maintains options which affect the resulting PDF.
///
/// Instances of this class are [ref_langspecific_t reference counted].
class IProfile
    : public IRefCounted
{
public:
    /// Sets a profile option.
    virtual void set(Char const* option, Char const* value) = 0;

    /// Saves the profile to a file.
    virtual void save_to_file(Char const* fname) const = 0;

protected:
    ~IProfile() {}
};



} //namespace jag

#endif //__CONFIGURATION_H_JG1642__

