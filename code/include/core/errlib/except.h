// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef EXCEPT_JG1529_H__
#define EXCEPT_JG1529_H__

#include <jagpdf/detail/errcodes.h>
#include <interfaces/stdtypes.h>
#include <string>
#include <boost/exception.hpp>

namespace jag
{
class exception;
void output_exception(exception const& exc_, std::ostream& stream);

//
// base class for all exceptions
//
typedef std::pair<unsigned,std::string> msg_info_t;
class exception
    : public boost::exception
    , public std::exception
{
    boost::shared_ptr<exception>  m_exc;
    unsigned                      m_msg_id;

protected:
    exception(msg_info_t const& msg_info, exception const* exc);
    exception(exception const& other);

public: // to be implemented by derived classes
    virtual unsigned errcode() const = 0;
    virtual std::auto_ptr<exception> clone() const = 0;

public:
    exception* next() const;
    void set_next(exception const& next);
    unsigned msg_id() const;
    ~exception() throw();
};



namespace detail
{
  extern char const msg_generic_fmt[];

  template<unsigned TErrorCode, char const* TFmt=msg_generic_fmt>
  class exception_template
      : public exception
  {
  public:
      explicit exception_template(msg_info_t const& msg_info, exception const* next=0);
      exception_template(exception_template const& other);
      unsigned errcode() const;
      std::auto_ptr<exception> clone() const;
      char const* what() const throw();
  };



  template<unsigned TErrorCode, class TDefaultMessage, char const*TFmt=msg_generic_fmt>
  class exception_template_with_message
      : public exception
  {
  public:
      explicit exception_template_with_message(msg_info_t const& msg_info, exception const* next=0);
      explicit exception_template_with_message(exception const* next=0);
      exception_template_with_message(exception_template_with_message const& other);

      unsigned errcode() const;
      std::auto_ptr<exception> clone() const;
      char const* what() const throw();
  };
} // namespace detail



//
// basic exception classes
// just typedefs, instantiated elsewhere
//


/// Should be used for any error related to input/ouput
/// processing. tag_io_object should be used to tag the object in
/// question.
typedef detail::exception_template<err_io_error> exception_io_error;


struct msg_invalid_operation;
/// Expresses that user does not use an interface according to
/// semantics described by documentation but not enforced by the
/// interfaces design.
typedef detail::exception_template_with_message<
    err_invalid_operation,
    msg_invalid_operation>
exception_invalid_operation;


struct msg_internal_error;
/// Should be used in case the program cannot reliably continue, in
/// particular:
/// - precondition/postcondition/invariant violation
/// - instruction pointer reached location that should be never executed
/// - initialization of fundamental modules failed
typedef detail::exception_template_with_message<
    err_internal_error,
    msg_internal_error> exception_internal_error;



/// Should be used for any error encoutered  while processing content of
/// an input file like font, image, etc.
typedef detail::exception_template<err_invalid_input> exception_invalid_input;

typedef detail::exception_template<err_invalid_value> exception_invalid_value;
typedef detail::exception_template<err_operation_failed> exception_operation_failed;

//
// exception tags
//

typedef boost::error_info<struct tag_errno, int> errno_info;
typedef boost::error_info<struct tag_user_errno, jag_Int> user_errno_info;
typedef boost::error_info<struct tag_msg, std::string> err_msg_info;
typedef boost::error_info<struct tag_io_object, std::string> io_object_info;

#ifdef _WIN32
typedef boost::error_info<struct tag_win_error, unsigned> win_error_info;
#endif

#ifndef JAG_DEBUG
  namespace detail
  {
    typedef boost::error_info<struct tag_no_loc_i,int> tag_no_loc;
  }
# define JAGLOC jag::detail::tag_no_loc(0)
#else
# define JAGLOC \
    boost::throw_function(BOOST_CURRENT_FUNCTION) <<    \
    boost::throw_file(__FILE__) <<  \
    boost::throw_line((int)__LINE__)
#endif

// BOOST_ERROR_INFO .. source code location


} // namespace jag


#endif // EXCEPT_JG1529_H__
/** EOF @file */
