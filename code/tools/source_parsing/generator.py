#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import sys
import os

def my_expandvars(text):
    if not os.getenv('JAGBASE_ROOT'):
        text = text.replace('$JAGBASE_ROOT/', '')
    return os.path.expandvars(text)
# point the interpreter to our packages
sys.path.insert(0, my_expandvars('$JAGBASE_ROOT/code/tools/external/python'))

import getopt
import introspect
import cgen
import cppgen
import traceback
import qbkgendoc
import re

g_license = """// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//
"""



rename_rex = re.compile( "%rename\s*\(([a-zA-Z_0-9]+)\s*\)\s*([a-zA-Z_0-9:]+)\s*;" )
def get_rename_map(cfg):
    result = {}
    for line in file( cfg['rename_def'] ):
        line = line.strip()
        if not line or line.startswith( '//' ):
            continue
        m = rename_rex.match( line )
        if not m:
            raise RuntimeError( "[rename] unrecognized: %s" % line )
        result[m.group(2)]=m.group(1)
    return result


ignore_rex = re.compile( "^%ignore\s*([a-zA-Z_0-9:]+)\s*;\s*$", re.M )
def get_ignored_set(cfg):
    return set( ignore_rex.findall( file( cfg['ignore_def'] ).read() ) )

typemap_rex = re.compile( "^//!\s*([a-zA-Z_0-9 ]+)\s*$", re.M )
def get_typemaps(cfg):
    result = []
    for tm in typemap_rex.findall(file( cfg['typemap_def'] ).read()):
        result.append( tm.split() )
    return result

def swig_ignore( cfg, intro ):
    """ignored methods, enum values, enums"""
    ignored_cls = []
    for cls in intro.exported_classes():
        for m in cls.internal_methods():
            ignored_cls.append( m )
    lines = list(set( [ "%%ignore %s;" % introspect.qualified_name( r.gcc ) for r in ignored_cls ] ))
    lines.sort()
    ignored = "\n".join( lines )
    ignored_evals = set()
    for enum in intro.exported_enums():
        for v in enum.ignored_names():
            ignored_evals.add( (introspect.qualified_name(enum.gcc.parent), v ) )
    lines = [ "%%ignore %s::%s;" % ( r[0], r[1] ) for r in ignored_evals ]
    lines.sort()
    ignored = ignored + "\n" + "\n".join( lines )
    return ignored

#
# Renames all types according to cpp_type_map. There are two categories of types:
#
#  - reference counted .. used with boost::intrusive_ptr<> in API; the
#                         instantiated intrusive_ptr is mapped to the name in
#                         the map, the base type is ignored and thus not
#                         exported
#  - raw pointer       .. just mapped using the map
#
def swig_rename_types(cfg, intro):
    lst = []
    m = cfg['cpp_type_map']
    for cls in intro.exported_classes():
        cls_name = introspect.qualified_name(cls.gcc)
        if cls_name not in cfg['ref_counted']:
            lst.append('%%rename(%s) %s;' % (m[cls_name], cls_name))
        else:
            lst.append('%%ignore %s;' % cls_name)
            lst.append('%%template(%s) boost::intrusive_ptr<%s>;' % (m[cls_name], cls_name))
    lst.sort()
    return '\n'.join(lst)


def swig( cfg, intro, opts ):
    ignored = swig_ignore( cfg, intro )
    #templates = swig_instantiate_templates( cfg, intro )
    renames = swig_rename_types(cfg, intro)
    contents = g_license
    contents = contents + '\n//-------\n'.join(['', ignored, renames, ''])
    open(cfg['swig_output'], "w").write(contents)


def c_wrapper( cfg, intro, opts ):
    cgen.main( cfg, intro )

def cpp_wrapper( cfg, intro, opts ):
    cppgen.main( cfg, intro )

def gendoc( cfg, intro, opts ):
    qbkgendoc.main( cfg, intro )

def usage():
    print usage_str
    sys.exit(2)

usage_str = """generator.py [options] <pyfile> <symbol>
options:
 -s, --swig-file      swig file
 -c, --c-api          c wrapper
 -x, --cpp-api        cpp wrapper
 -d                   documentation generator
 --api-header-dir     dir for api header files
 --api-impl-dir       dir for api impl files
 --api-swig-dir       dir for swig files
 --doc-dir            dir for generated documentation
 --gccxml-path        path to gccxml
 --gccxml-compiler    compiler to be simulated by gccxml
 --gccxml-cache       gccxml cache
"""



def parse_command_line():
    app_opts = { 'pyfile' : None,
                 'symbol' : None,
                 'action' : [],
                 'includes' : [],
                 'need_doxygen': False,
                 'api-header-dir': None,
                 'api-impl-dir' : None,
                 'api-swig-dir': None,
                 'doc-dir' : None,
                 'gccxml-path' : None,
                 'gccxml-compiler': None,
                 'gccxml-cache': None}

    try:
        opts, args = getopt.getopt( \
            sys.argv[1:], "scxdI:", \
            [ "swig-file", "c-api", "cpp-api", 'api-header-dir=', \
              'api-impl-dir=', 'api-swig-dir=', 'gccxml-path=', \
              'gccxml-compiler=', 'gccxml-cache=', 'doc-dir='])
        for opt, val in opts:
            if opt in ( '-s', '--swig-file' ):
                app_opts['action'].append( swig )
            elif opt in ('-c', '--c-api' ):
                app_opts['action'].append( c_wrapper )
            elif opt in ( '-x', '--cpp-api' ):
                app_opts['action'].append( cpp_wrapper )
            elif opt.startswith( '-I' ):
                #normalize \\ -> /
                val = val.replace('\\', '/')
                app_opts['includes'].append( val )
            elif opt.startswith( '-d' ):
                app_opts['action'].append(gendoc)
                app_opts['need_doxygen'] = True
            elif opt in ('--doc-dir'):
                app_opts['doc-dir'] = val
            elif opt in ('-h', '--api-header-dir'):
                app_opts['api-header-dir'] = val
            elif opt in ('-i', '--api-impl-dir'):
                app_opts['api-impl-dir'] = val
            elif opt in ('-s', '--api-swig-dir'):
                app_opts['api-swig-dir'] = val
            elif opt in ('--gccxml-path'):
                app_opts['gccxml-path'] = val
            elif opt in ('--gccxml-compiler'):
                app_opts['gccxml-compiler'] = val
            elif opt in ('--gccxml-cache'):
                app_opts['gccxml-cache'] = val
    except getopt.GetoptError, err:
        print err
        usage()
    except RuntimeError, exc:
        print exc
        sys.exit( -1 )

    if not app_opts['action'] or len(args) != 2:
        usage()

    if not os.path.isfile( args[0] ):
        raise RuntimeError( "File %s not found." % args[0] )
    app_opts['pyfile'] = args[0]
    app_opts['symbol'] = args[1]

    return app_opts

def this_script_dir():
    return os.path.dirname( os.path.abspath( sys.argv[0] ) )

# cmake: we need to specify directories for api generated files from the
# command line
def redir_apifiles(cfg, opts):
    def redir_files(dir_key, *keys):
        if opts[dir_key]:
            for key in keys:
                cfg[key] = os.path.join(opts[dir_key],
                                        os.path.basename(cfg[key]))
    redir_files('api-header-dir', 'c_header', 'cpp_header')
    redir_files('api-impl-dir', 'c_impl')
    redir_files('api-swig-dir',\
                'swig_output', 'rename_def', 'ignore_def', 'typemap_def')
    if opts['doc-dir']:
        cfg['doxyxmlout'] = opts['doc-dir']
        cfg['qbkoutdir'] = opts['doc-dir']

def main():
    opts = parse_command_line()
    #orig = os.getcwd()
    try:
        expr = open( opts['pyfile'], "rt" ).read().replace('\r','')
        #os.chdir( this_script_dir() )
        d = {}
        exec( expr, d )

        cfg = d[opts['symbol']]

        # originally, configurations did set the include directories;
        # now this information is passed on the cmd line
        assert not 'includes' in cfg
        cfg['includes'] = opts['includes']
        cfg['need_doxygen'] = opts['need_doxygen']
        cfg['gccxml-path'] = opts['gccxml-path']
        cfg['gccxml-compiler'] = opts['gccxml-compiler']
        cfg['gccxml-cache'] = opts['gccxml-cache']

        cfg['rename'] = get_rename_map(cfg)
        cfg['ignore'] = get_ignored_set(cfg)
        cfg['typemap_lst'] = get_typemaps(cfg)

        cfg['running-script-dir'] = this_script_dir()

        redir_apifiles(cfg, opts)

        intro = introspect.introspect( cfg )
        for action in opts['action']:
            action( cfg , intro, opts )
    finally:
        #os.chdir( orig )
        pass

if __name__ == "__main__":
    try:
        main()
    except RuntimeError, err:
        traceback.print_exc()
        print err
        sys.exit(1)

