// -*-c++-*-

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


%include exception.i

%header
%{
#include <pdflib/privateapi.h>
#include <core/errlib/except.h>
#include <sstream>

namespace jag
{
  class exception_msg_formatter
  {
      std::string m_str;
  public:
      explicit exception_msg_formatter( exception const& exc_ )
      {
          std::stringstream stream;
          format_exception_message( exc_, stream );
          m_str = stream.str();
      }

      char const* c_str() const
      {
          return m_str.c_str();
      }
  };
}

%}

