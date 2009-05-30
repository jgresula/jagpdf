# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

from pygccxml import parser
from pygccxml import declarations
import StringIO
import re


def global_namespace():
    #configure GCC-XML parser
    config = parser.config_t( gccxml_path= "c:/Progra~1/GCC_XML/bin/gccxml.exe",\
                              include_paths= ["e:/starteam/docplatform/nextrelease/code/common"] )
    #parsing source file
    decls = parser.parse( ['interf.h'], config )
    return declarations.get_global_namespace( decls )



def apply_type_metainfo_visitor( mi, visitor ):
    name = 'on_' + mi['category']
    if hasattr( visitor, name ):
        return getattr( visitor, name )( mi )
    else:
        return getattr( visitor, 'on_default_type' )( mi )

class _simple_sig_handler:
    def __init__( self, o ):
        self.o = o

    def write( self, str_ ):
        self.o.write( str_ )

    def on_name( self, name ):
        self.o.write( name )

    def on_link_start( self, lnk ):
        pass

    def on_link_end( self ):
        pass





class type_visitor:
    def __init__( self, lang, opts ):
        self.type_map = opts[lang+'_type_map']
        self.opts = opts

    def _map_type( self, mi ):
        t = mi['base_name'].rstrip('::')
        return self.type_map[t]

    def on_const_pointer( self, mi ):
        return self._map_type(mi) + ' const*'

    def on_default_type( self, mi ):
        return self._map_type(mi)

class java_type_visitor:
    def __init__(self, opts):
        self.type_map = opts['java_type_map']
        self.opts = opts

    def _map_type( self, mi ):
        t = mi['base_name'].rstrip('::')
        return self.type_map[t]

    def on_const_pointer( self, mi ):
        t = mi['base_name'].rstrip('::')
        if t == 'jag::Char':
            return 'String'
        else:
            return self._map_type(mi) + "``\[\]``"

    def on_default_type( self, mi ):
        return self._map_type(mi)



###########################################################################
class sig_base:
    def _output_linked_entity( self, mi, h, action, *args ):
        if self.is_doc:
            id_ = mi['gcc'].id()
            if id_:
                h.on_link_start( id_ )
        action( *args )
        if self.is_doc and id_:
            if id_:
                h.on_link_end()

    def _on_start_output(self, callable_, handler ):
        return True

    def get( self, m, handler_type=_simple_sig_handler ):
        sig = StringIO.StringIO()
        h = handler_type( sig )
        if not self._on_start_output( m, h ):
            return sig.getvalue()
        gcc = m.gcc
        freefun = isinstance( gcc, declarations.calldef.free_function_t )
        num_args = len( m.metainfo()[1:] )
        mi = m.metainfo()
        self._output_linked_entity( mi.return_type(), h, self._on_return_type, mi, h )
        self._on_callable_name( freefun, m, h )
        self._on_arglist_start( h )
        if not freefun:
            self._on_before_first_method_argument( m, h, num_args )
        self._on_arguments( mi[1:], h, num_args )
        self._on_arglist_end( h )
        if not freefun and gcc.has_const:
            self._on_const(h)
        self._on_sig_end(h)
        return sig.getvalue()

    def _on_arglist_start( self, h ):
        h.write( '(' )

    def _on_arglist_end( self, h ):
        h.write( ')' )

    def _on_callable_name( self, is_freefun, m, h ):
        h.on_name( m.name() )

    def _on_sig_end( self, h):
        pass

    def _on_before_first_method_argument( self, m, h, num_args ):
        pass

    def _on_const( self, h ):
        pass



class c_cpp_base( sig_base ):
    def __init__( self, type_, opts, is_doc, default_args ):
        self.opts = opts
        self.tv = type_visitor( type_, opts )
        self.is_doc = is_doc
        self.default_args = default_args

    def map_type( self, mi ):
        return apply_type_metainfo_visitor( mi, self.tv )

    def _output_type( self, mi, h, write_space=True ):
        self._output_linked_entity( mi, h, h.write, self.map_type(mi) )
        if write_space:
            h.write( ' ' )


    def _on_arguments( self, mis, h, num_args ):
        for i, mi in enumerate( mis ):
            self._output_type( mi, h )
            h.write( mi['argname'] )
            if self.default_args and mi['default_value']:
                if mi['category'] == 'const_pointer':
                    assert mi['default_value'] == '0'
                    h.write( '=0' )
                elif mi['category'] == 'interface':
                    #print ">> %s" % mi['default_value']
                    assert mi['default_value'].endswith( '()' ) or mi['default_value'].endswith( '( )' )
                    self._on_default_argument_interface( mi, h )
                elif mi['category'] == 'enum':
                    h.write('='+mi['default_value'].split('::')[-1])
                else:
                    assert not "unknown default argument type"
            if i<num_args-1:
                h.write( ', ' )

    def _on_sig_end( self, h):
        pass


###########################################################################
class cpp_signature( c_cpp_base ):
    def __init__( self, opts, is_doc=False ):
        # do support default arguments
        c_cpp_base.__init__( self, 'cpp', opts, is_doc, True )

    def _on_const( self, h ):
        h.write( ' const' )

    def _on_default_argument_interface( self, mi, h ):
        assert mi['category'] == 'interface'
        h.write( '=' )
        self._output_type( mi, h, False )
        h.write( '()' )

    def _on_return_type( self, cm, h ):
        cat = cm.return_type_category()
        if cat == 'void':
            key = self.is_doc and 'cpp_void_return' or 'cpp_void_return_noexc'
            h.write( self.opts[key] )
        elif cat in [ 'interface', 'other' ]:
            h.write( self.map_type( cm.return_type() ) )
        else:
            assert not "shouldn't get here"
        h.write( ' ' )

    def _on_callable_name( self, is_freefun, m, h ):
        orig_name = m.gcc.demangled.split( '(' )[0]
        try:
            name = m.opts['rename'][orig_name]
        except KeyError:
            name = m.name()
        h.on_name( name )


#
#
#
class java_signature( c_cpp_base ):
    def __init__( self, opts, is_doc=False ):
        # do support default arguments
        c_cpp_base.__init__( self, 'java', opts, is_doc, True )
        self.tv = java_type_visitor(opts)
        # ugly, it should come from the configuration
        self.rename_d = {'jag::IDocument::finalize': 'finalize_doc'}
        self.rename_d.update(self.opts['rename'])

    def _on_start_output(self, callable_, handler ):
        if callable_.qname() in self.opts['ignore']:
            handler.write( 'n/a' )
            return False
        return True

    def _on_arguments( self, mis, h, num_args ):
        num_default_args = 0
        for i, mi in enumerate( mis ):
            if self.default_args and mi['default_value']:
                h.write(' [')
                num_default_args += 1
            if i > 0:
                h.write(', ')
            self._output_type(mi, h)
            h.write( mi['argname'] )
        h.write(num_default_args * ']')

    def _on_return_type( self, cm, h ):
        cat = cm.return_type_category()
        if cat == 'void':
            key = self.is_doc and 'cpp_void_return' or 'cpp_void_return_noexc'
            h.write( self.opts[key] )
        elif cat in [ 'interface', 'other' ]:
            h.write( self.map_type( cm.return_type() ) )
        else:
            assert not "shouldn't get here"
        h.write( ' ' )

    def _on_callable_name( self, is_freefun, m, h ):
        orig_name = m.gcc.demangled.split( '(' )[0]
        try:
            name = self.rename_d[orig_name]
        except KeyError:
            name = m.name()
        h.on_name( name )





###########################################################################
class c_signature( c_cpp_base ):
    def __init__( self, opts, is_doc=False ):
        c_cpp_base.__init__( self, 'c', opts, is_doc, False )


    def _on_callable_name( self, is_freefun, m, h ):
        if is_freefun:
            h.on_name( self.opts['c_freefun_name'](m))
        else:
            h.on_name( self.opts['c_method_name'](m))

    def _on_before_first_method_argument( self, m, h, num_args ):
        cls_mi = m.cls_metainfo()
        self._output_linked_entity( cls_mi, h, h.write, self.map_type( cls_mi ) )
        h.write( ' hobj' )
        if num_args:
            h.write( ', ' )

    def _on_default_argument_interface( self, mi, h ):
        assert mi['category'] == 'interface'
        h.write( '=0' )

    def _on_return_type( self, cm, h ):
        cat = cm.return_type_category()
        if not self.is_doc:
            h.write( self.opts['export_tag'] + ' ' )
        if cat == 'void':
            h.write( self.opts['c_void_return'] )
        elif cat in [ 'interface', 'other' ]:
            h.write( self.map_type( cm.return_type() ) )
        else:
            assert not "shouldn't get here"
        if not self.is_doc:
            h.write( ' ' + self.opts['callspec'] )
        h.write( ' ' )

###########################################################################
class py_signature( sig_base ):
    def __init__( self, opts, is_doc=False ):
        self.is_doc=is_doc
        self.opts=opts
        self.default_args=True

    def _on_start_output(self, callable_, handler ):
        if callable_.qname() in self.opts['ignore']:
            handler.write( 'n/a' )
            return False
        return True

    def _on_callable_name( self, is_freefun, m, h ):
        orig_name = m.gcc.demangled.split( '(' )[0]
        try:
            name = m.opts['rename'][orig_name]
        except KeyError:
            name = m.name()
        h.on_name(name)

    def _output_linked_entity( self, mi, h, action, *args ):
        pass

    def _on_return_type( self, mi, h ):
        return #do nothing here
        id_ = mi.return_type()['gcc'].id()
        if not id_:
            return
        h.on_link_start( id_ )
        h.write( 'rettype')
        h.on_link_end()
        h.write( ' <-- ')

    def _on_arguments( self, mis, h, len_args ):
        ignore = set()
        for tm in self.opts['typemap_lst']:
            assert(len(tm)==2)
            ti = [i for i in range(len_args) if mis[i]['argname']==tm[0] and i+1<len_args]
            for i in ti:
                try:
                    if mis[i+1]['argname']==tm[1]:
                        ignore.add(i+1)
                except ValueError:
                    pass
        args_i=list(set(range(len_args)).difference(ignore))
        args_i.sort()
        len_args = len(args_i)
        closing_brackets = 0
        for i in args_i:
            if self.default_args and mis[i]['default_value']:
                if mis[i]['category'] == 'const_pointer':
                    assert mis[i]['default_value'] == '0'
                    h.write( ' [' )
                elif mis[i]['category'] == 'interface':
                    assert mis[i]['default_value'].endswith( '()' ) or mis[i]['default_value'].endswith( '( )' )
                    h.write( ' [' )
                elif mis[i]['category'] == 'enum':
                    h.write( ' [' )
                else:
                    assert not "unknown default argument type"
                closing_brackets += 1
            if i!=args_i[0]:
                h.write( ', ' )
            id_ = mis[i]['gcc'].id()
            if id_:
                h.on_link_start( id_ )
            h.write( mis[i]['argname'] )
            if id_:
                h.on_link_end()
        if closing_brackets:
            h.write(closing_brackets * ']')



def main():
    global_ns = global_namespace()
    docp = global_ns.namespace( "DocPlatform" )
    cls = docp.class_( "IStoreManager" )
    ns = '::Strs::DocPlatform::'
    #round up metainfo for methods
    minfo = []
    for m in cls.mem_funs( function=lambda m: not m.name.endswith( '_internal' ) ):
        assert m.virtuality == 'pure virtual'
        assert m.access_type == 'public'
        minfo.append( (m, method_metainfo( m, ns ) ) )
    #generate signatures for various languages
    for sig_type in [cpp_signature, c_signature, py_signature]:
        sig = sig_type( ns )
        print 10*'-'
        for m, mi in minfo:
            print sig.get_method( m, mi)

if __name__ == "__main__":
    main()





