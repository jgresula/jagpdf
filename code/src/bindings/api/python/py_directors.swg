// -*-c++-*-

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


//
// Implements reference counting of PyObject associated with a C++
// class that is wrapped using director. This protects a director from
// being garbage collected when it is referenced only from C++
// (i.e. there is no reference in Python).
//

%define JAG_DIRECTOR_ADDREF_RELEASE_METHODS( cls, addref_meth, release_meth )
%wrapper %{
void cls::addref_meth() {
     Swig::Director *d = SWIG_DIRECTOR_CAST( this );
     assert( d && "should never fail" );
     PyObject *self = d->swig_get_self();
     assert( self && "some deeper problem" );
     SWIG_PYTHON_THREAD_BEGIN_BLOCK;
     Py_INCREF(self);
     SWIG_PYTHON_THREAD_END_BLOCK;
}

void cls::release_meth() {
     Swig::Director *d = SWIG_DIRECTOR_CAST( this );
     assert( d && "should never fail" );
     PyObject *self = d->swig_get_self();
     assert( self && "some deeper problem" );
     SWIG_PYTHON_THREAD_BEGIN_BLOCK;
     Py_DECREF(self);
     SWIG_PYTHON_THREAD_END_BLOCK;
}

void cls::check_error()
{
    // no op, as SwIG is able to throw an exception in case director throws a
    // python exception
}
%}
%enddef

%module(directors="1") module

%define DIRECTOR_CLASS(cls)
%feature("director") cls;
JAG_DIRECTOR_ADDREF_RELEASE_METHODS( cls, AddRef, Release )
DIRECTOR_EXCEPTIONS(cls)
%enddef

DIRECTOR_CLASS(jag::apiinternal::StreamOut)
