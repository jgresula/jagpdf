// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef FILESO_JG2313_H__
# define FILESO_JG2313_H__

#include <boost/noncopyable.hpp>

namespace jag {
namespace jstd {

class FilePlatformSpecificBase
    : public boost::noncopyable
{
public:
    FILE*                 m_handle;
};


}} // namespace jag::jstd

#endif // FILESO_JG2313_H__
/** EOF @file */
