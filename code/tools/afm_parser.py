#!/usr/bin/env python
#
# $(Copyright)
# $(License)
#


"""
Implements the following functions:

gen_cpp_jagbase()  ... generates .h/.cpp files with metrics
encoding_status()  ... gives info about encodings and core fontd
"""

import glyphlist
import glob
import re
import string
import sys
import md5

class Bunch:
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

class Face:
    def __init__( self ):
        self.chars = []
        self.kern_pairs = []
        self.FontName = ""
        self.FullName = ""
        self.FamilyName = ""
        self.FontBBox = None #[llx lly urx ury]
        self.EncodingScheme = ""
        self.CharacterSet = ""
        self.CapHeight = 0
        self.XHeight = 0
        self.Ascender = 0
        self.Descender = 0
        self.UnderlinePosition = 0
        self.BuiltinEncoding = 0
        self.UnderlineThickness = 0
        self.ItalicAngle = 0
        self.IsFixedPitch = True
        self.Weight = 400 # 100-900
        self.StdHW = 0
        self.StdVW = 0
        self.md5 = md5.new()

    def finalize(self):
        if self.EncodingScheme == 'FontSpecific':
            # sort by unicode
            self.chars.sort( lambda l,r: cmp(l.code,r.code) )
        else:
            # sort by code
            self.chars.sort( lambda l,r: cmp(l.unicode,r.unicode) )

    def on_afm_line(self,line):
        """called for each input line"""
        self.md5.update( line )


class HandlerBase:
    def __init__( self, face ):
        self.face = face

    def on_line( self, s ):
        self.face.on_afm_line( s )
        self.process_line_( s )


class FontMetricsHandler(HandlerBase):
    def __init__( self, face, arg ):
        HandlerBase.__init__( self, face )

    def process_line_( self, s ):
        kwd, val = get_kwd_and_val(s)
        if kwd in set( ['FontName','FullName','FamilyName', 'EncodingScheme', 'CharacterSet'] ):
            setattr( self.face, kwd, val ) #verbatim
        elif kwd in set( [
            'CapHeight',
            'XHeight',
            'Ascender',
            'Descender',
            'UnderlinePosition',
            'UnderlineThickness',
            'StdHW', 'StdVW' ] ):
            setattr( self.face, kwd, int(val) )
        elif kwd in set( ['ItalicAngle'] ):
            setattr( self.face, kwd, float(val) )
        elif kwd == "FontBBox":
            self.face.FontBBox = [int(s) for s in val.split(" ") ]
            assert( len(self.face.FontBBox) == 4 )
        elif kwd == "IsFixedPitch":
            self.face.IsFixedPitch = val=='true' and True or False
        elif kwd == "Weight":
            #see: http://www.w3.org/TR/CSS21/fonts.html#font-boldness
            self.face.Weight = { 'Medium' : 400,
                                 'Normal' : 400,
                                 'Roman'  : 400,
                                 'Bold'   : 700,
                                 'Light'  : 300 }[val]
        elif kwd in set( ['Version', 'Notice', 'Comment', 'Characters'] ):
            pass #ignore
        elif kwd in set( ['MappingScheme', 'EscChar', 'IsBaseFont', 'VVector', 'IsFixedV', 'CharWidth'] ):
            assert not "unsupported keyword"
        else:
            print "kwd: ", kwd
            assert not "unknown keyword"



class CharMetricsHandler(HandlerBase):
    def __init__( self, face, arg ):
        HandlerBase.__init__( self, face )

    def process_line_( self, s ):
        l = [ item.strip().split( ' ', 1) for item in s.split(';')[:-1] ]
        rd = dict( l )
        bbox = [int(s) for s in rd['B'].split(" ") ]
        assert( len(bbox) == 4 )
        try:
            u = glyphlist.glyph_to_unicode_map[rd['N']]
        except:
            assert( self.face.EncodingScheme == 'FontSpecific' )
            u = 0

        self.face.chars.append( Bunch(unicode=u,
                                      code=int(rd['C']),
                                      widthx=int(rd['WX']),
                                      bbox=bbox) )

class KernDataHandler(HandlerBase):
    def __init__( self, face, arg ):
        HandlerBase.__init__( self, face )

    def process_line_( self, s ):
        assert not "should not get here"


class KernPairsHandler(HandlerBase):
    def __init__( self, face, arg ):
        HandlerBase.__init__( self, face )

    def process_line_( self, s ):
        kwd, name1, name2, kern = s.split( ' ' )
        assert( kwd == 'KPX' )
        u1 = glyphlist.glyph_to_unicode_map[name1]
        u2 = glyphlist.glyph_to_unicode_map[name2]
        self.face.kern_pairs.append( (u1, u2, int(kern) ) )


def get_kwd_and_val( line ):
        sp = line.split( " ", 1 )
        assert( len(sp) == 1 or len(sp) == 2 )
        if len(sp) == 1:
            return sp[0], None
        else:
            return sp

def get_handler_type( handler ):
    return globals()[handler+'Handler']

def font_name_to_id( fontname ):
    return re.sub( '[^a-zA-Z_]', '_', fontname )

def font_name_to_enum( fontname ):
    return "T1_" + font_name_to_id( fontname ).upper()


def process_afm( instream ):
    """processes single afm file"""
    handlers = []
    face = Face()
    for line in instream:
        line = line.strip()
        key, val = get_kwd_and_val( line )
        if key.startswith( 'Start' ):
            handlers.append( get_handler_type( key[5:] )(face, val) )
        elif key.startswith( 'End' ):
            last=handlers.pop()
            assert( last.__class__==get_handler_type(key[3:]) )
        else:
            handlers[-1].on_line( line )
    face.finalize()
    return face;


def process_afm_dir( dirname ):
    """non-recursively processes diretory of afm files"""
    faces = []
    for fname in glob.glob( dirname + '/*.afm' ):
        faces.append( process_afm( open(fname) ) )
    return faces

###########################################################################
# header generator

cpp_header_muster="""// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)

#ifndef T1STANDARD_FONTS_JAG
#define T1STANDARD_FONTS_JAG

#include <interfaces/stdtypes.h>
#include <resources/interfaces/typeface.h>

// this file was generated by afm_parser.py

namespace jag {
namespace resources {

struct t1s_glyph {
    jag::Int unicode;
    jag::Int code;
    jag::Int widthx;
};

struct t1s_face {
    const TypefaceMetrics FontMetrics;
    jag::Char const* FontName;
    jag::Char const* FullName;
    jag::Char const* FamilyName;
    jag::Char const* EncodingScheme;
    bool const IsBuiltinEncoding;
    jag::Char const* CharacterSet;
    const jag::Int UnderlinePosition;
    const jag::Int UnderlineThickness;
    const jag::Double ItalicAngle;
    const bool       IsFixedPitch;
    const jag::Int Weight;
    const jag::Int StdHW;
    const jag::Int StdVW;
    const int num_glyphs;
    const t1s_glyph* const glyphs;
    Hash16  Hash;
};

enum T1_ADOBE_STANDARD_FONTS{
    $ENUMS,
    T1_NUM_FACES
};
extern t1s_face const* g_adobe_standard_t1_faces[T1_NUM_FACES];

}} //namespace jag::resources

#endif
/** EOF @file */
"""


def do_cpp_header( faces, outs ):
    ENUMS = ",\n    ".join( [ font_name_to_enum(f.FontName) for f in faces ] )
    outs.write( string.Template(cpp_header_muster).substitute( locals() ) )


###########################################################################
# cpp generator
cpp_impl_muster="""// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)


#include \"precompiled.h\"
#include \"t1adobestandardfonts.h\"

// this file was generated by afm_parser.py on

// if face.EncodingScheme != FontSpecific then  glyphs sorted by unicode
//                                        else  glyphs sorted by code

namespace jag {
namespace resources {
namespace {
$FACE_DEFS
} // anonymous namespace

extern t1s_face const* g_adobe_standard_t1_faces[T1_NUM_FACES] = {
    $FACE_PTRS
};

}} //namespace jag::resources
/** EOF @file */
"""


def do_cpp_impl( faces, outs ):
    FACE_PTRS = ",\n    ".join( [ "&"+font_name_to_id(f.FontName) for f in faces ] )
    FACE_DEFS = []
    for face in faces:
        FACE_DEFS.append( do_cpp_impl_face(face) )
    FACE_DEFS = "\n".join( FACE_DEFS )
    outs.write( string.Template(cpp_impl_muster).substitute( locals() ) )



cpp_impl_face_muster="""
const int ${FACEID}_num_glyphs = $NUM_GLYPHS;
const t1s_glyph ${FACEID}_glyphs[${FACEID}_num_glyphs] = {
    ${GLYPHS_DEF}
};

const t1s_face  $FACEID = {
    {
        /* units */                  1000,
        /* bbox_xmin */              $FontBBox_xmin,
        /* bbox_ymin */              $FontBBox_ymin,
        /* bbox_xmax */              $FontBBox_xmax,
        /* bbox_ymax */              $FontBBox_ymax,
        /* baseline_distance */      $BaselineDistance,
        /* ascender */               $Ascender,
        /* descender */              $Descender,
        /* avg_width */              $AvgWidth,
        /* max_width */              $MaxWidth,
        /* missing_width */          $MissingWidth,
        /* cap height */             $CapHeight,
        /* xheight */                $XHeight
    },  /* font metrics */

    /* font name */              \"$FontName\",
    /* full name */              \"$FullName\",
    /* family name */            \"$FamilyName\",
    /* encoding scheme */        \"$EncodingScheme\",
    /* built-in encoding */      $BuiltinEncoding,
    /* char set */               \"$CharacterSet\",
    /* underline position */     $UnderlinePosition,
    /* underline thickness */    $UnderlineThickness,
    /* italic angle */           $ItalicAngle,
    /* is fixed pitch */         $IsFixedPitch,
    /* weight */                 $Weight,
    /* horizontal stem w */      $StdHW,
    /* vertical stem w */        $StdVW,
    /* num glyphs */             $NUM_GLYPHS,
    /* glyph metrics */          ${FACEID}_glyphs,
    /* hash */                   { $HASH }
};
"""

def calc_face_width_attrs( face ):
    AvgWidth, MaxWidth,  MissingWidth = 0, -1, -1
    for c in face.chars:
        AvgWidth += c.widthx
        if c.widthx > MaxWidth:
            MaxWidth = c.widthx
        if c.unicode == 32:
            MissingWidth = c.widthx
    AvgWidth = AvgWidth / len( face.chars )
    return locals()


def do_cpp_impl_face(face):
    FACEID = font_name_to_id( face.FontName )
    NUM_GLYPHS = len(face.chars)
    GLYPHS_DEF = []
    for i in range( 0, NUM_GLYPHS, 5 ):
        GLYPHS_DEF.append( ", ".join( ["{%d,%d,%d}" % (c.unicode, c.code, c.widthx)
                                      for c in face.chars[i:i+5]] ) )
    GLYPHS_DEF = ",\n    ".join(GLYPHS_DEF)
    locals().update( face.__dict__ )
    locals()['IsFixedPitch'] = locals()['IsFixedPitch'] and "true" or "false"
    locals()['BuiltinEncoding'] = locals()['EncodingScheme'] == 'FontSpecific' and "true" or "false"
    HASH = ", ".join( [ "0x%02x"%ord(b) for b in face.md5.digest() ] )
    locals().update( calc_face_width_attrs(face) )
    FontBBox_xmin = face.FontBBox[0]
    FontBBox_ymin = face.FontBBox[1]
    FontBBox_xmax = face.FontBBox[2]
    FontBBox_ymax = face.FontBBox[3]
    # taken from FreeType, t1objs.c
    BaselineDistance = 1000*12/10
    if BaselineDistance < locals()['Ascender']-locals()['Descender']:
        BaselineDistance = locals()['Ascender']-locals()['Descender']
    return string.Template(cpp_impl_face_muster).substitute( locals() )

AFM_DIR = '../../external/data/Core14_AFMs/'

def gen_cpp_jagbase():
    faces = process_afm_dir(AFM_DIR)
    if faces:
        do_cpp_header( faces, open('../src/resources/typeman/t1adobestandardfonts.h', 'wb' ) )
        do_cpp_impl( faces, open('../src/resources/typeman/t1adobestandardfonts.cpp', 'wb' ) )


#C 33 ; WX 600 ; N exclam ; B 202 -15 398 572 ;
def encoding_status():
    content = open(AFM_DIR + 'Courier-Bold.afm').read()
    names = re.compile('; N ([a-zA-Z]+) ;')
    core_names = set(names.findall(content))
    encodings = ['windows-1250', 'windows-1251', 'windows-1252', 'windows-1253']
    for enc in encodings:
        for i in xrange(128,256):
            try:
                c = unicode(chr(i), enc)
                assert len(c) == 1
                codepoint = ord(c[0])
                name = glyphlist.unicode_to_glyph(codepoint)
                if name not in core_names:
                    print enc, name, "0x%x" % codepoint
            except UnicodeDecodeError, err:
                print enc, err



if __name__ == "__main__":
    #encoding_status()
    gen_cpp_jagbase()


    #    faces = process_afm_dir( 'c:/Code/cpp/sandbox/jagbase/code/src/resources/typeman/Core14_AFMs/' )
#    do_cpp_header( faces, sys.stdout )
#    do_cpp_impl( faces, sys.stdout )
#     for face in faces:
#         print '>', font_name_to_id( face.FontName )
#    face = process_afm( open('c:/Code/cpp/sandbox/jagbase/code/src/resources/typeman/Core14_AFMs/Helvetica.afm') )

