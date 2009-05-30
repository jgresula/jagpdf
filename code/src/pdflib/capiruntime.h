// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef C_API_RUNTIME_JG858_H__
#define C_API_RUNTIME_JG858_H__

#include <pdflib/apifreefuns.h>
#include <jagpdf/detail/capi.h>
#include <core/errlib/except.h>

#include <interfaces/refcounted.h>
#include <interfaces/configuration.h>

#include <resources/interfaces/typeface.h>
#include <resources/interfaces/colorspaces.h>
#include <resources/interfaces/imageproperties.h>
#include <resources/interfaces/imagespec.h>
#include <resources/interfaces/imagemask.h>
#include <resources/interfaces/font.h>

#include <pdflib/interfaces/pdfwriter.h>
#include <pdflib/interfaces/canvas.h>
#include <pdflib/interfaces/docoutline.h>
#include <pdflib/interfaces/pdfpage.h>

#include <boost/intrusive_ptr.hpp>
#include <boost/utility/enable_if.hpp>
 #include  <boost/type_traits/is_base_of.hpp>

namespace jag {


void tls_set_error_info(exception const& exc);


//
//
//
template<class H>
H ptr2handle(IRefCounted* p)
{
    p->AddRef();
    return static_cast<H>(static_cast<void*>(p));
}

//
//
//
template<class H>
H ptr2handle(INotRefCounted* p)
{
    return static_cast<H>(static_cast<void*>(p));
}


//
//
//
template<class P, class H>
typename boost::enable_if<
    boost::is_base_of<IRefCounted, P>, P*>::type
handle2ptr(H h)
{
    return static_cast<P*>(static_cast<IRefCounted*>(static_cast<void*>(h)));
}


//
//
//
template<class P, class H>
typename boost::enable_if<
    boost::is_base_of<INotRefCounted, P>, P*>::type
handle2ptr(H h)
{
    return static_cast<P*>(static_cast<INotRefCounted*>(static_cast<void*>(h)));
}



} // namespace jag

#endif //C_API_RUNTIME_JG858_H__
/** EOF @file */
