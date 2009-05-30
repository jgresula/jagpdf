// -*-c++-*-

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#if !defined(SWIGPYTHON)
#error "SWIG API: Unsupported language."
#endif

// For some reason, the following methods cause this warning:
// 467. Overloaded declaration not supported (no type checking rule for 'type')
//
// However, the functions work (verified in apitests)
%warnfilter(467) jag::IDocument::shading_pattern_load_n;
%warnfilter(467) jag::ICanvas::text_o;
%warnfilter(467) jag::ICanvas::text_simple_o;

%include ../jagpdf_core_prologue.i


//
// Python can be compiled with two different internal unicode representations -
// UCS2 or UCS4. This is a compile-time decision so the particular python
// installation uses one or the other but not both.
//
// When compiling a python extension, the extension code can use PyUnicode_xxx
// functions. But these are only aliases to actual functions: PyUnicodeUCS2_xxx
// or PyUnicodeUCS4_xxx. So it can easily happen (e.g. our case) that the
// extension is built against UCS2 Pyton installation and thus does not work
// with UCS4 installation as PyUnicodeUCS2_xxx symbols used in the extension
// cannot be resolved.
//
// In our typemap code, only one such symbol is used:
// PyUnicode_AsUTF8String. The typemap code itself does not make any assumption
// about the UCS, it just wants to get a utf-8 string from a python unicode
// object. So it should not matter whether we use UCS2 or UCS4 version in
// run-time and thus we load the appropriate symbol dynamically.
//
// Applicable on linux. It seems that on windows UCS2 python is used
// exclusively.
//
%header
%{
    typedef PyObject* (*jag_PyUnicode_AsUTF8String_t)(PyObject*);
    static jag_PyUnicode_AsUTF8String_t jag_PyUnicode_AsUTF8String = 0;
%}

%init
%{
#if defined(JAG_WIN32)
    jag_PyUnicode_AsUTF8String = PyUnicode_AsUTF8String;
#else
    dlerror();
    jag_PyUnicode_AsUTF8String = (jag_PyUnicode_AsUTF8String_t)dlsym(RTLD_DEFAULT, "PyUnicodeUCS2_AsUTF8String");
    if (dlerror())
    {
        jag_PyUnicode_AsUTF8String = (jag_PyUnicode_AsUTF8String_t)dlsym(RTLD_DEFAULT, "PyUnicodeUCS4_AsUTF8String");
        if (dlerror())
        {
            fprintf(stderr, "JagPDF: Cannot find neither PyUnicodeUCS2_AsUTF8String nor PyUnicodeUCS4_AsUTF8String. Aborting()");
            abort();
        }
    }
    assert(jag_PyUnicode_AsUTF8String);
#endif
%}

// ---------------------------------------------------------------------------
//                           Exceptions


%define PYTHON_CUSTOM_EXCEPTION(msg)
    SWIG_PYTHON_THREAD_BEGIN_BLOCK;
    PyErr_SetString(g_jagpdf_exception_type, msg);
    SWIG_PYTHON_THREAD_END_BLOCK;
    SWIG_fail;
%enddef

%exception {
    try
    {
        $action
    }
    catch (jag::exception const& exc)
    {
        jag::exception_msg_formatter fmt(exc);
        PYTHON_CUSTOM_EXCEPTION(fmt.c_str());
    }
    catch (std::exception const& exc)
    {
        PYTHON_CUSTOM_EXCEPTION(exc.what());
    }
// temporarily, should be present only for directors, but it is not possible to
// bind the %exception to a particular class, see
// http://thread.gmane.org/gmane.comp.programming.swig/12284
    catch (Swig::DirectorException &)
    {
        SWIG_fail;
    }
}

%define DIRECTOR_EXCEPTIONS(cls)
 // TBD: intended only for director classes, see the comment above
%enddef


//
// Crate a custom 'jagpdf.Exception' exception type
//
%header
%{
    static PyObject* g_jagpdf_exception_type = 0;
%}

%init
%{
    SWIG_PYTHON_THREAD_BEGIN_BLOCK;
    g_jagpdf_exception_type = PyErr_NewException((char *)"jagpdf.Exception",
                                                 PyExc_RuntimeError,
                                                 NULL);
    assert(g_jagpdf_exception_type);
    Py_INCREF(g_jagpdf_exception_type);
    PyDict_SetItemString(d, "myException", g_jagpdf_exception_type);
    SWIG_PYTHON_THREAD_END_BLOCK;
%}

%pythoncode
%{
    Exception = _jagpdf.myException
%}


%include py_typemaps.swg
%include py_directors.swg
%include ../jagpdf_core.i
