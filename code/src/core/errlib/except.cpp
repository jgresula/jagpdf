// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/errlib/errlib.h>
#include <core/jstd/crt_platform.h>
#include <core/generic/assert.h>
#include <core/generic/unused.h>

#include <boost/io/ios_state.hpp>
#include <boost/version.hpp>
#include <iomanip>

#ifdef _WIN32
# include <windows.h>
#endif

namespace jag
{

//
// base exception class
//
exception::exception(msg_info_t const& msg_info, exception const* exc)
    : m_msg_id(msg_info.first)
{
    *this << err_msg_info(msg_info.second);

    if (exc)
    {
        std::auto_ptr<exception> cloned(exc->clone());
        m_exc.reset(cloned.release());
    }
}

exception::exception(exception const& other)
    : boost::exception(other)
    , std::exception(other)
{
    m_exc = other.m_exc;
    m_msg_id = other.m_msg_id;
}


void exception::set_next(exception const& next)
{
    std::auto_ptr<exception> cloned(next.clone());
    m_exc.reset(cloned.release());
}


exception* exception::next() const
{
    return m_exc.get();
}

unsigned exception::msg_id() const
{
    return m_msg_id;
}


exception::~exception() throw()
{
}

namespace
{

  // handles breaking change in boost.exception
  // https://svn.boost.org/trac/boost/changeset/52225/trunk/boost/exception/get_error_info.hpp
  template <class T>
  struct err_info
  {
#if BOOST_VERSION >= 103900
      typedef T const* type;
#else
      typedef boost::shared_ptr<T const> type;
#endif
  };

  
  //
  // message formatters
  //
  void msg_fmt_errno(exception const& exc, std::ostream& stream)
  {
      if (err_info<int>::type errnr = boost::get_error_info<errno_info>(exc))
      {
          const int BUFF_SIZE = 255;
          char buffer[BUFF_SIZE+1];
          buffer[BUFF_SIZE]=0;
          stream << "syserr: " << jstd::strerror(*errnr, buffer, BUFF_SIZE) << std::endl;
      }
  }


  void msg_fmt_user_errno(exception const& exc, std::ostream& stream)
  {
      if (err_info<jag_Int>::type errnr = boost::get_error_info<user_errno_info>(exc))
          stream << "usrerr: " << *errnr << std::endl;
  }


  void msg_fmt_getlasterror(exception const& exc, std::ostream& stream)
  {
#ifdef _WIN32
      if (err_info<unsigned>::type err = boost::get_error_info<win_error_info>(exc))
      {
          const int BUFF_SIZE = 255;
          char buffer[BUFF_SIZE+1];
          buffer[BUFF_SIZE]=0;
          ::FormatMessage(
              FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL,
              *err,
              0,
              buffer,
              BUFF_SIZE,
              0);
          stream << "syserr: " << buffer;
      }
#else
      detail::ignore_unused(exc);
      detail::ignore_unused(stream);
#endif
  }

  void msg_fmt_location(exception const& exc, std::ostream& stream)
  {
      err_info<char const*>::type throw_file = boost::get_error_info<boost::throw_file>(exc);
      err_info<int>::type throw_line = boost::get_error_info<boost::throw_line>(exc);
      if (throw_file && throw_line)
      {
          stream << "source: " << *throw_file << '(' << *throw_line << ')';
          if(err_info<char const*>::type throw_fn = boost::get_error_info<boost::throw_function>(exc))
              stream << ": " << *throw_fn;

          stream << std::endl;
      }
  }

  void msg_fmt_message(exception const& exc, std::ostream& stream)
  {

      if (err_info<std::string>::type msg = boost::get_error_info<err_msg_info>(exc))
      {
          boost::io::ios_flags_saver ifs(stream);

          stream << std::hex << std::setfill('0')
                 << std::setw(2) << exc.errcode()
                 << '-'
                 << std::setw(4) << exc.msg_id()
                 << ' ' << *msg << std::endl;
      }
  }

  void msg_fmt_generic(exception const& exc, std::ostream& stream)
  {
      msg_fmt_message(exc, stream);

      if (err_info<std::string>::type io_object = boost::get_error_info<io_object_info>(exc))
          stream << "io obj: " << *io_object << std::endl;

      msg_fmt_user_errno(exc, stream);
      msg_fmt_errno(exc, stream);
      msg_fmt_getlasterror(exc, stream);
      msg_fmt_location(exc, stream);
  }


} //anonymous namespace




///////////////////////////////////////////////////////////////////////////
namespace detail
{
  template<unsigned TErrorCode, char const* TFmt>
  exception_template<TErrorCode,TFmt>::exception_template(msg_info_t const& msg_info, exception const* next)
      : exception(msg_info, next)
  {
  }

  template<unsigned TErrorCode, char const* TFmt>
  unsigned exception_template<TErrorCode,TFmt>::errcode() const
  {
      return TErrorCode;
  }

  template<unsigned TErrorCode, char const* TFmt>
  std::auto_ptr<exception> exception_template<TErrorCode,TFmt>::clone() const
  {
      return std::auto_ptr<exception>(new exception_template(*this));
  }

  template<unsigned TErrorCode, char const* TFmt>
  exception_template<TErrorCode,TFmt>::exception_template(exception_template const& other)
      : exception(other)
  {
  }

  template<unsigned TErrorCode, char const* TFmt>
  char const* exception_template<TErrorCode,TFmt>::what() const throw()
  {
      return TFmt;
  }




  template<unsigned TErrorCode, class TDefaultMessage, char const*TFmt>
  exception_template_with_message<TErrorCode,TDefaultMessage,TFmt>::exception_template_with_message(msg_info_t const& msg_info, exception const* next)
      : exception(msg_info, next)
  {
  }

  template<unsigned TErrorCode, class TDefaultMessage, char const*TFmt>
  exception_template_with_message<TErrorCode,TDefaultMessage,TFmt>::exception_template_with_message(exception const* next)
      : exception(TDefaultMessage(), next)
  {
  }

  template<unsigned TErrorCode, class TDefaultMessage, char const*TFmt>
  exception_template_with_message<TErrorCode,TDefaultMessage,TFmt>::exception_template_with_message(exception_template_with_message const& other)
      : exception(other)
  {
  }

  template<unsigned TErrorCode, class TDefaultMessage, char const*TFmt>
  unsigned exception_template_with_message<TErrorCode,TDefaultMessage,TFmt>::errcode() const
  {
      return TErrorCode;
  }

  template<unsigned TErrorCode, class TDefaultMessage, char const*TFmt>
  std::auto_ptr<exception> exception_template_with_message<TErrorCode,TDefaultMessage,TFmt>::clone() const
  {
      return std::auto_ptr<exception>(new exception_template_with_message(*this));
  }

  template<unsigned TErrorCode, class TDefaultMessage, char const*TFmt>
  char const* exception_template_with_message<TErrorCode,TDefaultMessage,TFmt>::what() const throw()
  {
      return TFmt;
  }


  // message formatter selection strings returned by what()
  extern char const msg_generic_fmt[] = "1] generic error";

} // namespace detail


// forms message from an exception
void output_exception(exception const& exc_, std::ostream& stream)
{
    for(exception const*exc=&exc_; exc; exc=exc->next())
    {
        msg_fmt_generic(*exc, stream);
    }
}



// explicit instantiation
template class detail::exception_template<err_io_error>;         //exception_io_error
template class detail::exception_template_with_message<err_invalid_operation, msg_invalid_operation>; //exception_io_error
template class detail::exception_template_with_message<err_internal_error, msg_internal_error>; // exception_internal_error
template class detail::exception_template<err_invalid_input>; // exception_invalid_input
template class detail::exception_template<err_invalid_value>; // exception_invalid_value;
template class detail::exception_template<err_operation_failed>; // exception_operation_failed;

} // namespace jag




/** EOF @file */
