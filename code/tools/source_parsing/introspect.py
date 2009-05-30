# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import doxyxml
import smartptr
import os
import string
import sys
import re
import logging


from pygccxml import parser
from pygccxml import declarations
from pygccxml import utils
import pygccxml
assert pygccxml.__version__ >= '0.9.5'


def setup_pygccxml(options):
    """options dictionary keys:
       - infiles .. list of c++ files to process
       - includes .. list of c++ include directories
    """
    #logger
    log_level = logging.NOTSET # logging.NOTSET|DEBUG
    utils.loggers.gccxml.setLevel( log_level )
    utils.loggers.root.setLevel( log_level )
    utils.loggers.root.addHandler( logging.StreamHandler() )
    #trailing backslash quotes the following "
    includes = [inc.rstrip('\\') for inc in options['includes']]
    gccxml_path = options['gccxml-path']
    if not gccxml_path:
        gccxml_path = os.getenv("JAG_GCCXML_PATH")
    if None == gccxml_path:
        gccxml_path = "GCCXML not found"
    gccxml_compiler = None
    if options['gccxml-compiler']:
        gccxml_compiler = options['gccxml-compiler']
    #configure GCC-XML parser
    config = parser.config_t( gccxml_path= gccxml_path,\
                              compiler = gccxml_compiler, \
                              include_paths= includes,\
                              undefine_symbols=['__MINGW32__'],
                              define_symbols=[])
    return config


def global_namespace( options ):
    config = setup_pygccxml(options)
    gcccache = options['gccxml-cache']
    if not gcccache:
        gcccache = 'gcccache'
    cache = parser.directory_cache.directory_cache_t(dir=gcccache, md5_sigs=False)
    # normalize input files so that cache can match them
    # mixed slashes does not play well with the cache
    in_files = [ os.path.abspath(p) for p in options['infiles'] ]
    decls = parser.parse( in_files, config, cache=cache )
    return declarations.get_global_namespace( decls )


re_gccatrs = re.compile('gccxml\(([^)]+)\)')
class typebase(object):
    def __init__( self, gcc, doxy, doxyindex, opts ):
        self.gcc = gcc
        self.doxy = doxy
        self.doxyindex = doxyindex
        self.opts = opts
        self.exported_ = True
        self.documented_ = True
        # processes comma separated list of attributes
        if hasattr(gcc, 'attributes') and \
               gcc.attributes and \
               gcc.attributes.startswith('gccxml'):
            gcc_attrs = set(re_gccatrs.match(gcc.attributes).group(1).split(','))
            gcc_attrs = [item.strip() for item in gcc_attrs]
            gcc_attrs = self.process_gcc_attrs(gcc_attrs)
            assert set(['internal', 'undocumented']).issuperset(gcc_attrs)
            self.exported_ = not 'internal' in  gcc_attrs
            self.documented_ = not 'undocumented' in  gcc_attrs

    def process_gcc_attrs(self, attrs):
        return attrs

    def is_exported( self ):
        return self.exported_

    def is_publicly_documented(self):
        return self.documented_

    def get_export_dependencies( self ):
        return []

    def __str__( self) :
        return self.gcc.decl_string

    def __repr__( self) :
        return self.gcc.decl_string

    def __eq__( self, other):
        return self.gcc == other.gcc

    def __hash__( self):
        return self.gcc.__hash__()

    def name( self ):
        return self.gcc.name

    def qname( self ):
        name = []
        this = self.gcc
        while this:
            name.append( this.name )
            this = this.parent
        name.reverse()
        return '::'.join(name[1:])

    def detailed_desc(self, handler):
        return self.doxy.detailed_desc(handler)

    def brief_desc(self, handler):
        return self.doxy.brief_desc(handler)

    def parameters_desc(self, handler):
        return self.doxy.parameters_desc(handler)

    def simplesect_desc(self, handler, kind):
        return self.doxy.simplesect_desc(handler, kind)

    def custom_alias_desc(self, handler, alias):
        return self.doxy.custom_alias_desc(handler, alias)

    def id( self ):
        if self.doxy:
            return self.doxy.id()
        return None



class callablebase(typebase):
    def __init__( self, gcc, doxy, doxyindex, opts ):
        typebase.__init__( self, gcc, doxy, doxyindex, opts )

    def get_export_dependencies( self ):
        result = set()
        for dep in self.gcc.i_depend_on_them():
            d = create_decl( dep.depend_on_it, self.doxyindex, self.opts )
            if d.is_exported():
                result.add( d )
        return result



class enumeration_i( typebase ):
    def __init__( self, gcc, doxyindex, opts ):
        doxy = doxyindex and doxyindex.find_enumeration(gcc.decl_string) or None
        self.all_values = None
        typebase.__init__( self, gcc, doxy, doxyindex, opts )
        if not self.all_values: # enum has not gcc attributes, call it here
            self.process_gcc_attrs([])
            assert self.all_values # empty enum
        self.minfo = None

    # Collect attributes for enum values.
    def process_gcc_attrs(self, attrs):
        retval = []
        self.internal = set()
        undocumented = set()
        for attr in attrs:
            if attr.startswith('internal_enum_val'):
                self.internal.update(set(attr.split()[1:]))
            elif attr.startswith('undocumented_enum_val'):
                undocumented.update(set(attr.split()[1:]))
            else:
                retval.append(attr)
        self.all_values = [v for v in self.gcc.values]
        all_names = set([n for n, v in self.all_values])
        assert all_names.issuperset(self.internal)
        assert all_names.issuperset(undocumented)
        self.public_values = [(n, v) for n, v in self.all_values if n not in self.internal]
        self.doced_values = [(n, v) for n, v in self.public_values if n not in undocumented]
#         print '>>?', self.qname()
#         print '>>?  all:', self.all_values
#         print '>>?  public:', self.public_values
#         print '>>?  documented_values:', self.doced_values
        return retval

    def values(self):
        """tuples [(name, val), ...]"""
        return self.public_values

    def documented_values(self):
        """tuples [(name, val), ...]"""
        return self.doced_values

    def ignored_names( self ):
        return list(self.internal)

    def object_by_id( self, id_ ):
        """return itself even if id represents a value """
        if self.id() == id_:
            return self
        for name, val in self.values():
            ev = self.doxy.value( name )
            if ev.id() == id_:
                return self

    def metainfo( self ):
        if not self.minfo:
            self.minfo = type_metainfo( self.gcc,
                                        self.opts,
                                        self.doxyindex,
                                        dict( argname=None ))
        return self.minfo


class fundamental_i( typebase ):
    def __init__( self, gcc, doxyindex, opts ):
        typebase.__init__( self, gcc, None, doxyindex, opts )

    def is_exported( self ):
        return False


class memfun_i( callablebase ):
    def __init__( self, gcc, doxyindex, opts ):
        self.minfo = None
        self.cls_minfo = None
        doxy = doxyindex and doxyindex.find_class(gcc.parent.decl_string).method(gcc.name) or None
        callablebase.__init__( self, gcc, doxy, doxyindex, opts )
        self.gcc_leaf_class = None

    def metainfo( self ):
        if not self.minfo:
            self.minfo = create_args_metainfo( self, self.opts, self.doxyindex )
        return self.minfo

    def remapped_name( self ):
        """retrieves remapped name (if any), otherwise falls back to self.name()"""
        try:
            name = self.opts['rename'][self.qname()]
        except KeyError:
            name = self.name()
        return name


    def cls_metainfo( self ):
        assert self.gcc_leaf_class
        if not self.cls_minfo:
            self.cls_minfo = type_metainfo( self.gcc_leaf_class,
                                            self.opts,
                                            self.doxyindex,
                                            dict( argname=None ),
                                            True )
        return self.cls_minfo


class freefun_i( callablebase ):
    def __init__( self, gcc, doxyindex, opts ):
        self.minfo = None
        p = gcc.parent
        ns = []
        while p:
            ns.insert( 0, p.name )
            p = p.parent
        ns = '::' + "::".join( ns[1:] ) + '::'
        doxy = doxyindex and doxyindex.find_function(ns + gcc.name) or None
        callablebase.__init__( self, gcc, doxy, doxyindex, opts )

    def object_by_id( self, id_ ):
        if self.id() == id_:
            return self

    def metainfo( self ):
        if not self.minfo:
            self.minfo = create_args_metainfo( self, self.opts, self.doxyindex )
        return self.minfo


def iter_seq( iters ):
    for it in iters:
        for item in it:
            yield item

class class_i( typebase ):
    def __init__( self, gcc, doxyindex, opts ):
        doxy = doxyindex and doxyindex.find_class( gcc.decl_string ) or None
        typebase.__init__( self, gcc, doxy, doxyindex, opts )
        self.methods_ = set()
        self.internal_methods_ = set()

    def _process_methods( self ):
        if not self.methods_:
            query = declarations.access_type_matcher_t( 'public' )
            hierarchy = self.gcc.recursive_bases
            cls_decls = self.gcc.decls(function=query)
            base_decls = [ c.related_class.decls( function=query, allow_empty=True ) for c in hierarchy if c.access_type == 'public']
            for m in iter_seq( [cls_decls] + base_decls ):
                if isinstance( m, declarations.enumeration.enumeration_t ):
                    try:
                        e = create_decl(m, self.doxyindex, self.opts)
                    except RuntimeError:
                        pass # ok - not found by doxygen mechanism
                    else:
                        if e.is_exported():
                            raise RuntimeError( "Enum '%s' within class '%s' - not allowed." % (m.name, self.gcc.name) )
                elif isinstance( m, declarations.calldef.member_function_t ):
                    if m.virtuality != 'pure virtual':
                        raise RuntimeError( "'%s' is not virtual" % m )
                    dm = create_decl( m, self.doxyindex, self.opts )
                    if dm.is_exported():
                        self.methods_.add( dm )
                    else:
                        self.internal_methods_.add( dm )
                    # this is needed as methods in base classes need to know
                    # the class they belong to in the generated (flat) hierarchy
                    dm.gcc_leaf_class = self.gcc
                else:
                    if not isinstance( m, ( declarations.calldef.constructor_t,
                                            declarations.calldef.destructor_t,
                                            declarations.calldef.member_operator_t) ):
                        raise RuntimeError( "Unknown declaration '%s' of type '%s' within class '%s'." % (m.name, m, self.gcc.name) )

    def methods( self ):
        self._process_methods()
        return self.methods_

    def internal_methods( self ):
        self._process_methods()
        return self.internal_methods_

    def get_export_dependencies( self ):
        result = set()
        for m in self.methods():
            [result.add(d) for d in m.get_export_dependencies()]
        return result

    def object_by_id( self, id_ ):
        if self.id() == id_:
            return self
        for m in self.methods():
            if m.id() == id_:
                return m


def qualified_name( gcc ):
    s = []
    while gcc:
        s.append( gcc.name )
        gcc = gcc.parent
    s.reverse()
    return "::".join( s[1:] )



###########################################################################
# metainfo generation - dictionary with the following keys
# - type     - pygccxml type
# - category - one of [const_pointer,void,enum,other,interface]
# - base     - base type - for a smartpointer it is its value_type
#              otherwise has the same value as type keyword
# - typemap  - number of the following arguments that are bound to this one
#              i.e. a normal argument has this set to 0
def type_metainfo( type_, opts, doxyindex, result, allow_raw_cls=False ):
    result['type'] = type_
    dec_traits = declarations.type_traits
    # interfaces
    if opts['smartptr_traits'].match( type_ ):
        # intrusive_ptr interfaces
        value_type = opts['smartptr_traits'].value_type( type_ )
        assert value_type.decl_string[2:] in opts['ref_counted']
        result = metainfo_for_class( value_type, opts, doxyindex, result )
        result['ref_counted'] = True
    elif allow_raw_cls and dec_traits.is_pointer(type_) and dec_traits.is_class_declaration(type_.base):
        # raw pointer interfaces
        assert not dec_traits.is_const(type_.base)    # pointer must not be const
        decl_string = type_.base.decl_string
        cls_decl = dec_traits.remove_declarated(type_.base)
        r = cls_decl.top_parent.decls( name=decl_string,
                                       function=lambda decl: not isinstance(decl, declarations.class_declaration.class_declaration_t),
                                       allow_empty=True)
        assert decl_string[2:] not in opts['ref_counted']
        if len(r) != 1:
            raise RuntimeError("Unable to find out class for declaration %s" % decl_string)
        result = metainfo_for_class(r[0], opts, doxyindex, result)
        result['ref_counted'] = False
    else:
        if dec_traits.is_reference( type_ ):
            raise RuntimeError( "%s: no references allowed" % type_ )
        # pointers  (string, arrays)
        if dec_traits.is_pointer( type_ ):
            assert dec_traits.is_const( type_.base )    # pointer must be const
            result['category'] = 'const_pointer'
            result['base'] = type_.base.base
        else:
            assert not dec_traits.is_const( type_ )     # other types cannot be const
            if dec_traits.is_void( type_ ):
                result['category'] = 'void'
            elif dec_traits.is_enum( type_ ):
                result['category'] = 'enum'
            else:
                result['category'] = 'other'
            result['base'] = type_
    return postprocess_metainfo( opts, doxyindex, result )

def metainfo_for_class( cls, opts, doxyindex, result ):
    """provides metainfo for a class - low level"""
    assert cls.decl_string.lstrip('::').startswith( opts['cls_namespace'] )   # interfaces only in specified namespace
    result['category'] = 'interface'
    result['base'] = normalize_decl(cls)
    assert declarations.type_traits.is_class( result['base'] )
    return result

def postprocess_metainfo( opts, doxyindex, result ):
    """finalizes a metainfo by adding some generic fields"""
    result['base_name'] = result['base'].decl_string.lstrip( '::' )
    assert result['base'] and result['type'] and result['category']
    result['gcc'] = create_decl( result['base'], doxyindex, opts )
    return result


def apply_type_metainfo_visitor( mi, visitor ):
    name = 'on_' + mi['category']
    return getattr( visitor, name )( mi )

# def create_type_metainfo( type_, cat, base, argname ):
#     return dict( type=type_,\
#                  category=cat,\
#                  base=base,\
#                  argname=argname,\
#                  base_name=base.decl_string )


class ArgsMetainfo:
    def __init__( self, arg_list, callable_ ):
        self.arg_list = arg_list    #list of arguments as a type metainfo (dict)
        self.callable_ = callable_  #our xxx_i type object

    def __getitem__(self, i):
        return self.arg_list[i]

    def return_type(self):
        return self.arg_list[0]

    def args(self):
        return self.arg_list[1:]

    def callable_object(self):
        return self.callable_

    def is_memfun(self):
        return type(self.callable_) == memfun_i

    def return_type_category(self):
        cat = self.return_type()['category']
        if cat in [ 'void', 'interface', 'other' ]:
            return cat
        elif cat == 'enum':
            return 'other'
        elif cat == 'const_pointer' and\
            self.return_type()['base_name'] in self.callable_.opts['allowed_const_pointers_as_ret_types']:
            return 'other'
        print cat
        raise RuntimeError( 'unsupported return type: %s - %s'\
                            % ( self.return_type()['base_name'],
                                self.callable_.gcc.name ) )


def create_args_metainfo( callable_, opts, doxyindex ):
    """provides a metainfo for aruments of a callable"""
    allowed_pointers = opts['allowed_pointers']
    m = callable_.gcc
    result = []
    result.append( type_metainfo( m.return_type, opts, doxyindex, dict( argname=None ), True ) )
    for arg in m.arguments:
        if re.compile('arg[0-9]+').match( arg.name ):
            raise RuntimeError( "%s: unnamed argument: %s" % (m.name, arg ) )
        d = dict( argname=arg.name, default_value = arg.default_value )
        mi = type_metainfo( arg.type, opts, doxyindex, d, True )
        if mi['category'] == 'const_pointer' and mi['base_name'] not in allowed_pointers:
            raise RuntimeError( "%s: disallowed pointer type: %s" % (m.name, mi['base_name']) )
        result.append( mi )
    #find typemaps, adds 'typemap' key to each minfo
    #  number of following arguments forming a typemap (0 if no typemap is detected)
    def match( t, mi ):
        for k, v in t.iteritems():
            if mi[k] != v: return False
        return True

    typemaps = opts['typemaps']

    args = result[1:]
    for i, minfo in enumerate( args ):
        args[i]['typemap'] = 0
        for typemap in typemaps:
            tmap_found = True
            start_i = i
            for t in typemap:
                if not match(t,args[start_i]):
                    tmap_found = False
                    break
                start_i += 1
            if tmap_found:
                args[i]['typemap'] = start_i - i - 1
                break

    return ArgsMetainfo(result, callable_)




def create_decl( gcc, doxy, opts ):
    # strip type, remove smart pointer
    result = None
    gcc_ = gcc
    gcc = declarations.type_traits.remove_declarated(gcc)
    gcc = declarations.type_traits.remove_alias(gcc)
    gcc = declarations.type_traits.remove_pointer(gcc)
    gcc = declarations.type_traits.remove_reference(gcc)
    gcc = declarations.type_traits.remove_cv(gcc)
    if opts['smartptr_traits'].match( gcc ):
        gcc = opts['smartptr_traits'].value_type( gcc )
    gcc = declarations.type_traits.remove_declarated(gcc)
    gcc = normalize_decl( gcc )


    if gcc.decl_string in opts['passthrough_types']:
        result = fundamental_i( gcc, doxy, opts )
    elif declarations.type_traits.is_class(gcc):
        result = class_i( gcc, doxy, opts )
    elif declarations.type_traits.is_enum(gcc):
        result = enumeration_i( gcc, doxy, opts )
    elif declarations.type_traits.is_fundamental(gcc):
        result = fundamental_i( gcc, doxy, opts )
    elif isinstance( gcc, declarations.calldef.free_function_t ):
        result = freefun_i( gcc, doxy, opts )
    elif isinstance( gcc, declarations.calldef.member_function_t ):
        result = memfun_i( gcc, doxy, opts )
    else:
        raise RuntimeError( 'unrecognized declaration %s - %s' % (gcc_.decl_string, type(gcc_) ) )
    assert result
    return result



def is_class( gcc ):
    return isinstance( gcc, (declarations.class_declaration.class_declaration_t,\
                             declarations.class_declaration.class_t ))



def normalize_decl( gcc ):
    #do more caching
    if isinstance( gcc, declarations.class_declaration.class_declaration_t ):
    #if is_class( gcc ):
        try:
            return gcc.top_parent.decl( name=gcc.decl_string,\
                                        function=lambda d: isinstance(d, declarations.class_declaration.class_t) )
        except declarations.matcher.declaration_not_found_t:
            raise RuntimeError( "No definition found for %s" % gcc.decl_string )
    return gcc



def _create_export_set( ns, type_str_list, doxy, opts ):
    to_analyze = set()
    result = set()
    for type_str in type_str_list:
        to_analyze.add( create_decl(ns.decl( name=type_str ), doxy, opts ) )

    while len(to_analyze):
        decl = to_analyze.pop()
        result.add( decl )
        [ to_analyze.add(d) for d in decl.get_export_dependencies() if d not in result]

    # some enums are not found as they are not referenced from
    # signatures e.g. masks that are passed through UInt
    for enum_name in opts['additional_enums']:
        pass

    return result



def _run_doxygen(infiles, xmloutdir, cfg):
    """infiles - list of c++ files to process,
       xmloutdir - doxygen destination directory
    returns doxyxml.Doxy, doxygen output"""
    doxy_template = os.path.join(cfg['running-script-dir'], 'doxyfile-template')
    tmpl = string.Template(open(doxy_template).read())
    subst_dict = dict( jg_input=' \\\n\t'.join([ '"%s"' % f for f in infiles]),\
                       doxyxmlout='"%s"' % xmloutdir )
    doxycfg = tmpl.safe_substitute( subst_dict )
    return doxyxml.get_comments( doxycfg, xmloutdir )


###########################################################################
# public
class introspect:
    def __init__( self, options, run_doxy=True ):
        """options dictionary keys:
           - infiles .. list of c++ files to process
           - includes .. list of c++ include directories
           - entryp .. list of top-level declarations to export
           - doxyxmlout .. destination directory for doxygen xml
        """
        self.opts = options
        self.doxyindex_ = None
        self.expset = None
        self.run_doxy = run_doxy
        self.enums = None
        self.freefuns = None
        self.classes = None

    def doxyindex( self ):
        if not self.opts['need_doxygen']:
            return None
        if not self.doxyindex_:
            if self.run_doxy:
                self.doxyindex_, out = _run_doxygen( self.opts['infiles'],\
                                                     self.opts['doxyxmlout'],\
                                                     self.opts)
            else:
                self.doxyindex_ = doxyxml.Doxy( self.opts['doxyxmlout'] )
                out = None
            if out:
                print '---[doxygen]---'
                print out
                print '---------------'
        return self.doxyindex_

    def export_set( self ):
        if not self.expset:
            ns = global_namespace( self.opts )
            input_symbols = self.opts['entryp'] + self.opts['additional_enums']
            self.expset = _create_export_set( ns, input_symbols, self.doxyindex(), self.opts )
            for item in self.expset:
                assert isinstance( item, (class_i, enumeration_i, freefun_i) )
        return self.expset

    def exported_classes( self ):
        if self.classes == None:
            self.classes = [ item for item in self.export_set() if isinstance(item, class_i ) ]
        return self.classes

    def exported_enums( self ):
        if None == self.enums:
            # for some reason pygccxml gives several instances of the same
            # enumeration in some cases
            enums=[ item for item in self.export_set() if isinstance(item, enumeration_i ) ]
            unique = {}
            for enum in enums:
                unique[enum.gcc.decl_string]=enum
            self.enums = [ e for e in unique.itervalues() ]
        return self.enums

    def exported_freefunctions( self ):
        if self.freefuns == None:
            self.freefuns = [ item for item in self.export_set() if isinstance(item, freefun_i ) ]
        return self.freefuns

    def object_by_id( self, id_ ):
        for obj in self.export_set():
            found = obj.object_by_id(id_)
            if found:
                return found
        assert not "id not found"



###########################################################################
# configurations
def _doc_cls_name( obj ):
    assert declarations.type_traits.is_class( obj )
    assert obj.name[0] == 'I'
    return obj.name[1:]

def _c_method_name( obj ):
    assert isinstance( obj, declarations.calldef.member_function_t )
    return '_'.join( ['c', obj.parent.name, obj.name ] )


def _c_cls_name( obj ):
    assert declarations.type_traits.is_class( obj )
    assert obj.name[0] == 'I'
    return 'c_' + obj.name


def get_test_w():
    includes = [ os.path.expandvars( p ) for p in [ "c:/Code/boost-versions/boost_1_34_0" ] ]
    infiles = ['interf.h', 'incomplete.h']
    entryp = ['::Strs::DocPlatform::IStoreManager',\
               '::Strs::DocPlatform::factory']
    xmlout = 'xml-test'
    qbkoutdir = os.path.expandvars( 'c:/Code/python/sig/qbk' )
    ns = 'Strs::DocPlatform::'
    remove_from_links = [ns]
    allowed_pointers=[ ns+p for p in ['Int', 'Char'] ]
    typemaps = [ ( dict(base_name=ns+'Int', category='const_pointer', argname='array_x' ),\
                   dict(base_name=ns+'Int', category='other', argname='length' ) )\
                 ]
    smartptr_traits = smartptr.intrusive_ptr_traits
    cpp_type_map =  dict( [ ('void', 'result_t') ] +\
                          [ (ns + src, tgt ) for src, tgt in [('Char','Strs::char'),\
                                                              ('Int', 'Strs::int'),\
                                                              ('Double', 'Strs::double' ) ] ] )

    c_type_map =  dict( [ ('void', 'cres') ] +\
                          [ (ns + src, tgt ) for src, tgt in [('Char','c_char'),\
                                                              ('Int', 'c_int'),\
                                                              ('Double', 'c_double' ) ] ] )

    return dict( includes=includes,\
                 infiles=infiles,\
                 entryp=entryp,\
                 doxyxmlout=xmlout,\
                 qbkoutdir=qbkoutdir,\
                 remove_from_links=remove_from_links,\
                 doc_cls_name = _doc_cls_name,\
                 allowed_pointers = allowed_pointers,\
                 typemaps = typemaps,\
                 cls_namespace=ns,\
                 smartptr_traits=smartptr_traits,\
                 cpp_type_map=cpp_type_map,\
                 cpp_cls_name=_doc_cls_name,\
                 c_type_map=c_type_map,\
                 c_cls_name=_c_cls_name,\
                 c_method_name=_c_method_name)


def get_test_cfg():
    includes = [ os.path.expandvars( p ) for p in [ "$BOOST_ROOT" ] ]
    infiles = ['interf.h', 'incomplete.h']
    entryp = ['::Strs::DocPlatform::IStoreManager',\
              '::Strs::DocPlatform::factory']
    xmlout = 'xml-test'
    qbkoutdir = os.path.expandvars( '$JAGBASE_ROOT/doc/quickbook/gendoc' )

    ns = 'Strs::DocPlatform::'
    remove_from_links = [ns]
    allowed_pointers=[ ns+p for p in ['Int', 'Char'] ]
    typemaps = [ ( dict(base_name=ns+'Int', category='const_pointer', argname='array_x' ),\
                   dict(base_name=ns+'Int', category='other', argname='length' ) )\
                 ]
    smartptr_traits = smartptr.intrusive_ptr_traits
    cpp_type_map =  dict( [ ('void', 'result_t') ] +\
                          [ (ns + src, tgt ) for src, tgt in [('Char','Strs::char'),\
                                                              ('Int', 'Strs::int'),\
                                                              ('Double', 'Strs::double' ) ] ] )
    c_type_map =  dict( [ ('void', 'cres') ] +\
                          [ (ns + src, tgt ) for src, tgt in [('Char','c_char'),\
                                                              ('Int', 'c_int'),\
                                                              ('Double', 'c_double' ) ] ] )

    return dict( includes=includes,\
                 infiles=infiles,\
                 entryp=entryp,\
                 doxyxmlout=xmlout,\
                 qbkoutdir=qbkoutdir,\
                 remove_from_links=remove_from_links,\
                 doc_cls_name = _doc_cls_name,\
                 allowed_pointers = allowed_pointers,\
                 typemaps = typemaps,\
                 cls_namespace=ns,\
                 smartptr_traits=smartptr_traits,\
                 cpp_type_map=cpp_type_map,\
                 cpp_cls_name=_doc_cls_name,\
                 c_type_map=c_type_map,\
                 c_cls_name=_c_cls_name,\
                 c_method_name=_c_method_name)








if __name__ == "__main__":
    opts = get_test_cfg()
#    opts = get_test_w()
    rtti = introspect( opts )
#     print 'cls:', rtti.object_by_id('class_strs_1_1_doc_platform_1_1_i_store_manager')
#     print 'meth:', rtti.object_by_id('class_strs_1_1_doc_platform_1_1_i_store_manager_143c5c24cbbba1ce3cab602c71d7e71d1')
#     print 'enum:', rtti.object_by_id('namespace_strs_1_1_doc_platform_156ae224539915fcd298471ccfdc8b6b6')
#     print 'enumval:', rtti.object_by_id('namespace_strs_1_1_doc_platform_121b0ca4da77b133f9e5fa5343e75876d480f14c9c8a9273ccd3e016fd1aee227')
#     print 'fun:', rtti.object_by_id('namespace_strs_1_1_doc_platform_165611b21a331545c7a53b8d5ff0fd47b')
    for cls in rtti.exported_classes():
        for m in cls.methods():
            print m.name(), m.metainfo()
    for fn in rtti.exported_freefunctions():
        print fn.name(), fn.metainfo()
    sys.exit(0)

    desp = rtti.export_set()
    for d in desp:
        if d.gcc.name == 'IStoreManager':
            for m in d.methods():
                if m.gcc.name == 'init':
                    base = HandlerChain( BaseHandler )
                    dump = HandlerChain( DumpHandler )
                    m.brief_desc( base )
                    m.brief_desc( dump )
                    m.detailed_desc( base )
#                    m.detailed_desc( dump )
                    print '===\n', base.output()
                    print '===\n', dump.output()
                    open( os.path.join( opts['qbkoutdir'], 'sandbox.qbk'), 'w' ).write(base.output())






