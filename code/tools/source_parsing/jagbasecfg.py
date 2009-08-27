# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

from pygccxml import declarations
import os
import smartptr
import re
import introspect
import re

# the current limitiation is dependency on JAGBASE_ROOT env var that
# directory could be looked up by traversing up from the current
# directory

def my_expandvars(text):
    if not os.getenv('JAGBASE_ROOT'):
        text = text.replace('$JAGBASE_ROOT/', '')
    return os.path.expandvars(text)

rex_infile = re.compile( '^\s*EXPORT_FILE\(\s*[<"] *([^>"]+) *[>"]\s*\)', re.MULTILINE )
def get_files_to_process():
    def_file = my_expandvars( '$JAGBASE_ROOT/code/src/bindings/api/files.swg' )
    return rex_infile.findall(open( def_file).read())

class TypeMap:
    def __init__( self ):
        self.maps = {}
        self.maps['c'] = self._get_map( 'c' )
        self.maps['cpp'] = self._get_map( 'cpp' )
        self.maps['java'] = self._get_map('java')

    def get_map( self, lang ):
        return self.maps[lang]

    def _get_map( self, lang ):
        ## !!! it looks like the third column is used only for handle
        ## name, but not when a function name is generated
        map_ = """
        #- passthrough
        jag_streamout; StreamOut;  jag_streamout; StreamOut

        #- fundamental
        void;          void;     void;        ????
        jag::Int;      Int;      jag_Int;     int
        jag::Char;     Char;     jag_Char;    ????
        jag::UInt;     UInt;     jag_UInt;    long
        jag::Double;   Double;   jag_Double;  double
        jag::Byte;     Byte;     jag_Byte;    byte
        jag::UInt8;    UInt8;    jag_UInt8;   ????
        jag::UInt16;   UInt16;   jag_UInt16;  int

        #- classes
        jag::ICanvas;            Canvas;                jag_Canvas;    Canvas
        jag::IDocument;          Document;              jag_Document;  Document
        jag::IPage;              Page;                  jag_Page;      Page

        jag::IImageDef;          ImageDef;               jag_ImageDef;   ImageDef
        jag::IImageMask;         ImageMask;              jag_ImageMask;  ImageMasd
        jag::IImage;             Image;                  jag_Image;      Image
        jag::IFont;              Font;                   jag_Font;       Font
        jag::IProfile;           Profile;                jag_Profile;    Profile
        jag::IDocumentOutline;   DocumentOutline;        jag_DocumentOutline; DocumentOutline

        #- resource ids
        jag::ColorSpace;             ColorSpace;         jag_ColorSpace;  ColorSpace
        jag::Pattern;                Pattern;            jag_Pattern;     Pattern
        jag::Function;               Function;           jag_Function;    Function
        jag::ImageMaskID;            ImageMaskID;        jag_ImageMaskID; ImageMaskID
        jag::Destination;            Destination;        jag_Destination; Destination

        #- enumerations
        jag::PatternTilingType;      PatternTilingType;     jag_PatternTilingType;   PatternTilingType
        jag::RenderingIntentType;    RenderingIntentType;   jag_RenderingIntentType; RenderingIntentType
        jag::LineCapStyle;           LineCapStyle;          jag_LineCapStyle;        LineCapStyle
        jag::LineJoinStyle;          LineJoinStyle;         jag_LineJoinStyle;       LineJoinStyle
        jag::ImageFormat;            ImageFormat;           jag_ImageFormat;         ImageFormat
        jag::ColorSpaceType;         ColorSpaceType;        jag_ColorSpaceType;      ColorSpaceType
        """

#        jag::FaceType;               FaceType;              jag_FaceType
#         jag::EnumCharacterEncoding;  EnumCharacterEncoding; jag_EnumCharacterEncoding
#


        result = {}
        index = { 'cpp' : 1,\
                  'c'   : 2,\
                  'java' : 3}[lang]
        for t in [ s.strip() for s in map_.split( '\n' ) ]:
            t = t.strip()
            if not t or t[0]=='#': continue
            cols = [ s.strip() for s in t.split(';') ]
            assert cols[0] not in result
            result[cols[0]] = cols[index]
        return result


g_type_map = TypeMap()


def _doc_cls_name(obj):
    assert declarations.type_traits.is_class( obj )
    return obj.name[1:]

def _c_method_name(obj):
    assert isinstance( obj, introspect.memfun_i )
    name = obj.cls_metainfo()['type'].name
    assert name.startswith('I') # strip leading I from interface name
    return 'jag_%s_%s' % ( name[1:], obj.gcc.name )

def _c_freefun_name(obj):
    assert isinstance( obj, introspect.freefun_i )
    return 'jag_' + obj.gcc.name

def _c_enum_value(name):
    return 'jag_' + name


###########################################################################
# jagpdf configuration
g_license = """/*
 *  Copyright (c) 2005-2009 Jaroslav Gresula
 *
 *  Distributed under the MIT license (See accompanying file
 *  LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
 */
"""
def _true_cfg():
    in_ = get_files_to_process()
    inlist = [ os.path.join(my_expandvars("$JAGBASE_ROOT/code/include" ), i ) for i in in_ ]

    c_header_prologue = """#ifndef C_API_HEADER_H_JAGPDF__
#define C_API_HEADER_H_JAGPDF__
#include <jagpdf/detail/c_prologue.h>

#ifdef __cplusplus
extern \"C\"
{
#endif
"""
    c_header_epilogue = """

#ifdef __cplusplus
} /* extern \"C\" */
#endif
#endif
"""
    cpp_header_prologue = """
#ifndef CPP_API_HEADER_JAGPDF_H__
#define CPP_API_HEADER_JAGPDF_H__
#include <jagpdf/detail/cpp_prologue.h>

namespace jag {
namespace pdf {
"""
    cpp_header_epilogue = """
}} // namespace jag::pdf

#endif
"""

    return dict( infiles=inlist,
                 # api files - if changing adjust also output files in
                 # pdflib/CMakeLists.txt
                 c_header = my_expandvars('$JAGBASE_ROOT/code/include/jagpdf/detail/capi.h'),
                 c_impl = my_expandvars('$JAGBASE_ROOT/code/src/pdflib/capiimpl.cpp'),
                 cpp_header = my_expandvars('$JAGBASE_ROOT/code/include/jagpdf/detail/cppapi.h'),
                 swig_output = my_expandvars('$JAGBASE_ROOT/code/src/bindings/api/generated.swg'),
                 rename_def = my_expandvars('$JAGBASE_ROOT/code/src/bindings/api/rename.swg'),
                 ignore_def = my_expandvars('$JAGBASE_ROOT/code/src/bindings/api/ignore.swg'),
                 typemap_def = my_expandvars('$JAGBASE_ROOT/code/src/bindings/api/typemapdef.swg'),
                 #
                 qbkoutdir=my_expandvars( '$JAGBASE_ROOT/doc/quickbook/' ),
                 doxyxmlout = my_expandvars('$JAGBASE_ROOT/stage/xml-jagbase'),


                 c_impl_prologue = g_license + '#include "precompiled.h"\n#include "capiruntime.h"\nusing namespace jag;\n\n',
                 c_header_prologue = g_license + c_header_prologue,
                 c_header_epilogue = c_header_epilogue,
                 cpp_header_prologue = g_license + cpp_header_prologue,
                 cpp_header_epilogue = cpp_header_epilogue,

                 c_cls_handle_gen = 'JAG_GEN_UNIQUE_HANDLE(%s);\n',
                 c_handle_inc='jag_addref',
                 c_handle_dec='jag_release',
                 no_exceptions_tag='JAG_DO_NOT_USE_EXCEPTIONS',
                 exception_instance='Exception()',
                 internal_exception_cls='jag::exception',
                 c_catch_set_global_error= 'jag::tls_set_error_info( exc )',

                 c_method_name=_c_method_name,
                 c_freefun_name=_c_freefun_name,
                 c_enum_value = _c_enum_value,
                 cpp_enum_value = lambda x: x,
                 c_void_return='jag_error',
                 cpp_void_return='void',
                 cpp_void_return_noexc='Result',
                 export_tag='JAG_EXPORT',
                 callspec = 'JAG_CALLSPEC',
                 err_ret_value=lambda x: "%s()" % x,
                 global_error_check='jag_error_code()',
                 allowed_const_pointers_as_ret_types=['jag::Char'],
                 cpp_cls_order={ 'jag::IFont' : 10,
                                 'jag::IImage' : 20,
                                 'jag::ICanvas' : 30,
                                 'jag::IImageDef' : 40,
                                 'jag::IDocument' : 1000000 },
                 additional_enums=['::jag::ColorSpaceType'],
                 passthrough_types=['::jag_streamout_tag'],
                 ref_counted=['jag::IProfile',
                              'jag::IDocument',
                              'jag::IImageMask'])




###########################################################################
# keywords in the configuration
#
# infiles                     ... source files to be processed by parser
# qbkoutdir                   ... where to put generated quickbook files
# doxyxmlout                  ... where to put doxy generated files


def cfg( testenv=False):
    result_d = testenv and 1 or _true_cfg()

    entry_points =     [ "::jag::create_profile",\
                         "::jag::create_file",\
                         '::jag::create_stream',\
                         "::jag::create_profile_from_string",\
                         "::jag::create_profile_from_file",\
                         "::jag::version",\
                       ]
    ns = 'jag::'
    remove_from_links = [ns]
    allowed_pointers=[ ns+p for p in ['Char', 'Double', 'UInt', 'Byte', 'Int', 'Function', 'UInt16'] ] + ['jag_streamout']
    def get_typemap( p, pname ):
        return ( dict(base_name=p, category='const_pointer', argname=pname ),\
                 dict(base_name='UInt', category='other', argname='length' ) )
    typemaps = [ get_typemap(p,p=='Char' and 'text' or 'array_in') for p in allowed_pointers ]
    smartptr_traits = smartptr.intrusive_ptr_traits

    result_d.update(dict( entryp = entry_points,
                          remove_from_links=remove_from_links,
                          doc_cls_name = _doc_cls_name,
                          allowed_pointers = allowed_pointers,
                          typemaps = typemaps,
                          cls_namespace=ns,
                          smartptr_traits=smartptr_traits,
                          cpp_type_map=g_type_map.get_map( 'cpp' ),
                          c_type_map=g_type_map.get_map( 'c' ),
                          java_type_map = g_type_map.get_map('java')) )
    return result_d


#test_config = cfg(True)
jagpdf_config = cfg(False)


if __name__ == "__main__":
    get_files_to_process()
