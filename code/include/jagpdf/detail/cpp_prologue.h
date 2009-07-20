/*
 * Copyright (c) 2005-2009 Jaroslav Gresula
 *
 * Distributed under the MIT license (See accompanying file
 * LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
 *
 */
#ifndef CPP_PROLOGUE_JG2222_H__
#define CPP_PROLOGUE_JG2222_H__

#include <jagpdf/detail/capi.h>

#ifndef JAG_DO_NOT_USE_EXCEPTIONS
# include <exception>
# include <string.h>
#endif


namespace jag {
namespace pdf {

typedef UInt ColorSpace;
typedef UInt Pattern;
typedef UInt ImageMaskID;
typedef UInt Destination;
typedef UInt Function;

#ifndef JAG_DO_NOT_USE_EXCEPTIONS
typedef void Result;
typedef jag_error ErrorCode;


namespace detail
{
  class exception_impl
  {
      ErrorCode   m_code;
      int         m_refcount;
      Char*       m_msg;

  public:
      exception_impl()
          : m_code(jag_error_code())
          , m_refcount(0)
          , m_msg(0)
      {
          Char const* start=jag_error_msg();
          Char const* end=start;
          while(*++end);
          m_msg = new Char[end-start+1];
          memcpy(m_msg, start, end-start+1);
      }

      void addref() {
          m_refcount++;
      }

      void release() {
          if (!--m_refcount)
              delete this;
      }

      ErrorCode code() const {
          return m_code;
      }

      Char const* message() const {
          return m_msg;
      }

  private:
      ~exception_impl() {
          delete [] m_msg;
      }
  };
}
//
//
//
class Exception
    : public std::exception
{
    detail::exception_impl* m_impl;
public:
    Exception()
        : m_impl(new detail::exception_impl)
    {
        m_impl->addref();
    }

    Exception(Exception const& other)
        : std::exception() // to get rid of a gcc warning
    {
        m_impl=other.m_impl;
        m_impl->addref();
    }

    Exception& operator=(Exception const& other) {
        m_impl->release();
        m_impl=other.m_impl;
        m_impl->addref();
        return *this;
    }

    const char* what() const throw() {
        return m_impl->message();
    }

    ErrorCode code() const {
        return m_impl->code();
    }

    ~Exception() throw() {
        m_impl->release();
    }
};
#else
  typedef jag_error Result;
#endif


//
//
//
class StreamOut
    : public jag_streamout
{
public:
    StreamOut() {
        jag_streamout::write = StreamOut::s_write;
        jag_streamout::close = StreamOut::s_close;
        jag_streamout::custom_data = this;
    }

    virtual ~StreamOut() {}

protected:
    virtual Int write(void const* data, ULong size) = 0;
    virtual Int close() = 0;

private:
    static jag_Int JAG_CALLSPEC s_write(void* this_, void const* data, jag_ULong size) {
        return static_cast<StreamOut*>(this_)->write(data, size);
    }

    static jag_Int JAG_CALLSPEC s_close(void* this_) {
        return static_cast<StreamOut*>(this_)->close();
    }
};


}} // namespace jag::pdf

#endif
/** EOF @file */
