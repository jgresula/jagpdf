// -*-c++-*-

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


// %typemap(in) (jag::Char const* text, jag::UInt length ) {
//     $1 = PyString_AsString($input);   /* char *str */
//     $2 = PyString_Size($input);       /* int len   */
// }


//
// The following typemap transform python sequences to (ptr,length)
// pair.
//
%define BUFFER_OR_SEQUENCE_TO_ARRAY( type, getter )
%typemap(in) (type const* array_in, jag::UInt length ) (bool delete_buffer=false)
{
    // !! The order of checks must be preserved
    // Otherwise PySequence_Check() succeeds on array.array and might
    // set wrong buffer size (num of items instead of byte size)
    if ( PyObject_CheckReadBuffer($input) )
    {
        void const* tmp=0;
        Py_ssize_t tmp2=0;
        if ( -1==PyObject_AsReadBuffer( $input, &tmp, &tmp2) )
            PyErr_SetString(PyExc_ValueError,"Cannot access the buffer");

        $1 = static_cast<type*>( const_cast<void*>(tmp) );
        $2 = static_cast<jag::UInt>( tmp2/sizeof(type) );
    }
    else if ( PySequence_Check($input) )
    {
        $2 = PySequence_Length($input);
        if ($2)
        {
            delete_buffer = true;
            $1 = (type*) malloc($2*sizeof(type));
            for (jag::UInt i = 0; i < $2; i++)
            {
                PyObject *o = PySequence_GetItem($input,i);
                if (PyNumber_Check(o))
                {
                    $1[i] = (type) getter(o);
                }
                else
                {
                    PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");
                    free($1);
                    return NULL;
                }
            }
        }
        else
        {
            $1=0;
        }
    }
    else
    {
        PyErr_SetString(PyExc_ValueError,"Expected a sequence or a buffer");
        return NULL;
    }
}
%typemap(freearg) (type const* array_in, jag::UInt length) {
   if (delete_buffer$argnum) free($1);
}
// needed when the argument is in a overloaded function/method
%typemap(typecheck) (type const* array_in, jag::UInt length) {
    $1 = PySequence_Check($input) || PyObject_CheckReadBuffer($input) ? 1 : 0;
}
%enddef

BUFFER_OR_SEQUENCE_TO_ARRAY( jag::Double, PyFloat_AsDouble )
BUFFER_OR_SEQUENCE_TO_ARRAY( jag::UInt, PyInt_AsLong )
BUFFER_OR_SEQUENCE_TO_ARRAY( jag::Byte,   PyInt_AsLong )
BUFFER_OR_SEQUENCE_TO_ARRAY( jag::Int, PyInt_AsLong )
BUFFER_OR_SEQUENCE_TO_ARRAY( jag::Function, PyInt_AsLong )
BUFFER_OR_SEQUENCE_TO_ARRAY( jag::UInt16, PyInt_AsLong )


//
// apply typemap to different argument names, for instance we need to
// handle a case where the typemap is applied twice in a single
// function signature
//
%apply (jag::Int const* array_in, jag::UInt length ) {
    (jag::Int const* positions, jag::UInt positions_length)
};

%apply (jag::Double const* array_in, jag::UInt length ) {
    (jag::Double const* offsets, jag::UInt offsets_length)
};



//
// The following typemap does not copy any memory, it just wraps
// passed memory with a python buffer.
//
%typemap(directorin)(void const* data_in, jag::ULong length) {
  $input=PyBuffer_FromMemory(const_cast<void*>($1_name),$2_name);
}



//
// Intended for a single-byte encoding argument. If an unicode object
// is passed then it is converted to utf-8.
//
// This conversion incurs an extra python object thus it is prefered
// to pass python utf-8 encoded strings if performance matters.
//
%typemap(in) jag::Char const* txt_u (PyObject* pytmp=0) {
  if (PyString_Check($input)) {
    $1=PyString_AsString($input);
  }
  else if (PyUnicode_Check($input)) {
    pytmp = jag_PyUnicode_AsUTF8String($input);
    Py_INCREF(pytmp);
    $1=PyString_AsString(pytmp);
  }
  else {
    PyErr_SetString(PyExc_ValueError,"Expected a string");
    return NULL;
  }
}
%typemap(freearg) jag::Char const* txt_u {
  if (pytmp$argnum)
    Py_DECREF(pytmp$argnum);
}
// needed when the argument is in a overloaded function/method
%typemap(typecheck) jag::Char const* txt_u {
  $1 = (PyString_Check($input) || PyUnicode_Check($input)) ? 1 : 0;
}
