// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef TSS_JG233_H__
#define TSS_JG233_H__

#include <boost/shared_ptr.hpp>

namespace jag {
namespace jstd {


/**
 * @brief Interface that performs TLS cleanup
 */
class ITlsCleanup
{
public:
    virtual void operator()(void *data) = 0;
protected:
    ~ITlsCleanup() {}
};




namespace detail
{

  /// Generic ITlsCleanup implementation
  template<class T>
  class TlsCleanupImpl
      : public ITlsCleanup
  {
      void operator()(void *data) {
          delete static_cast<T*>(data);
      }
  };
}




/// Convenience function that creates a clenup function for given type.
template<class T>
boost::shared_ptr<ITlsCleanup> make_tls_cleanup()
{
    return boost::shared_ptr<ITlsCleanup>(new detail::TlsCleanupImpl<T>);
}




/**
 * @brief Set thred specific data
 *
 * @param key      cookie for this data
 * @param value    value to stored
 * @param cleanup  cleanup function
 */
void set_tls_data(unsigned key, void* value, boost::shared_ptr<ITlsCleanup> cleanup);


/**
 * @brief Get thread specific data
 *
 * @param key   cookie
 * @return      thread specific value
 */
void* get_tls_data(unsigned key);


/**
 * @brief Must be defined by a main module if TLS is used.
 *
 * This is just to ensure that the main module invokes tls_on_<...>()
 * hooks.
 */
void tls_cleanup_implemented();

//
// tss slot ids
//
const unsigned tss_id_error_record = 1;


//
// The following hooks must be implemented by the main module in case
// of tls use
//
bool tls_on_process_attach();
bool tls_on_process_detach();
#ifdef _WIN32
bool tls_on_thread_attach();
bool tls_on_thread_detach();
#endif



namespace detail
{
  //
  // Platform independent representation of tls slot content
  //
  struct TlsSlotData;
  struct TlsSlotData
  {
    TlsSlotData*                     m_next;
    unsigned                         m_key;
    void*                            m_ptr;
    boost::shared_ptr<ITlsCleanup>   m_cleanup;
  };


  typedef std::pair<detail::TlsSlotData*,detail::TlsSlotData*> HeadAndFound;
  TlsSlotData* get_tls_data_head();
  void set_tls_data_head(void* data);
  HeadAndFound find_tls_entry(unsigned key);
  void tls_slot_cleanup(void* ptr);

} // namespace detail

}} // namespace jag::jstd

#endif // TSS_JG233_H__
/** EOF @file */
