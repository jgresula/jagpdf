#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import sig
import introspect
from pygccxml import declarations

def get_streams( cfg, *stems ):
    """writes appropriate prologue for each stream (if any)"""
    files = {}
    for stem in stems:
        f = open( cfg[stem], 'wb' )
        prologue = stem + '_prologue'
        if prologue in cfg:
            f.write( cfg[prologue] )
        files[stem]=f
    return files

def finish_streams( cfg, streams ):
    """finishes stream and writes appropriate epilogue or EOF """
    for stem, stream in streams.iteritems():
        epilogue = stem + '_epilogue'
        if epilogue in cfg:
            stream.write( cfg[epilogue] )
        else:
            stream.write( '\n' )

def output_enums( header, intro, cfg, lang ):
    """writes exported enums to the header stream"""
    visitor = sig.type_visitor( lang, cfg )
    val_fn = cfg[lang+'_enum_value']
    header.write( '\n/* ==== enums ==== */\n' )
    enums = [(sig.apply_type_metainfo_visitor(e.metainfo(), visitor), e) \
             for e in intro.exported_enums()]
    enums.sort()
    for enum_name, enum in enums:
        items = [ "  %s=%s" % (val_fn(name), val) for name, val in enum.values() ]
        header.write( "typedef enum {\n%s\n} %s;\n\n" % ( ',\n'.join( items ), enum_name ))


def map_type( type_mi, lang, cfg ):
    vis = sig.type_visitor( lang, cfg )
    return sig.apply_type_metainfo_visitor( type_mi, vis )

def decl_string_generic(cls):
    assert isinstance(cls, (introspect.class_i,
                            declarations.class_declaration.class_t))
    if type(cls) == introspect.class_i:
        return cls.gcc.decl_string.lstrip('::')
    elif type(cls) == declarations.class_declaration.class_t:
        return cls.decl_string.lstrip('::')
    assert not 'should not get here'

def cls_from_cls( cls, opts ):
    return _map_class( cls, opts, 'cpp' )

def c_handle_from_cls( cls, opts ):
    return _map_class( cls, opts, 'c' )

def _map_class( cls, opts, lang ):
    mp = opts[lang+'_type_map']
    return mp[decl_string_generic(cls)]

def update_dict( d, src, *keys ):
    for k in keys:
        d[k]=src[k]
