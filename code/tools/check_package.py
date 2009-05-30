#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

"""check_package.py --type=<cpp,py> file1 file2 ...
"""

import sys
import platform
import getopt
import fnmatch
import os
import popen2
import re
import tempfile
import glob

class NumOkFiles: pass
num_ok = NumOkFiles()


class Error(Exception):
    def __init__(self, msg):
        self.msg = msg


def is_unix():
    return -1 == platform.platform().lower().find( 'windows' )

def make_pipe_unix(cmd, ignore_retcode=False):
    pipe = popen2.Popen4(cmd)
    pipe.tochild.close()
    out=pipe.fromchild.read()
    retcode=pipe.wait()
    if retcode and not ignore_retcode:
        raise Error('FAILED(%d): %s\n%s' % (retcode,cmd,out))
    return out

def make_pipe_win(cmd, ignore_retcode=False):
    outs, ins = popen2.popen4(cmd)
    ins.close()
    out=outs.read()
    retcode=outs.close()
    if retcode and not ignore_retcode:
        raise Error('FAILED(%d): %s\n%s' % (retcode,cmd,out))
    return out

def make_pipe(cmd, ignore_retcode=False):
    pipe = is_unix() and make_pipe_unix or make_pipe_win
    return pipe(cmd,ignore_retcode)

class TempFile:
    def __init__(self):
        handle, self.name = tempfile.mkstemp()
        os.close(handle)
    def close(self):
        if os.path.isfile(self.name):
            os.unlink(self.name)
    def __del__(self):
        self.close()

allowed_dlls = [ "kernel32.dll", "gdi32.dll", "user32.dll", "advapi32.dll", "python2[3-6].dll" ]
allowed_win_exports = [ "icudt.?.?_dat", "jag_*", 'init_jagpdf', '_Java_com_jagpdf_jagpdfJNI_.+']

required_glibc = '@GLIBC_2.3'
allowed_unix_symbols = ( 'jag_.+', 'JAGPDF_[0-9]+\.[0-9]+', 'init_jagpdf', 'Java_com_jagpdf_jagpdfJNI_.+' )
allowed_unix_undef_symbols = ('@GLIBC', '__tls_get_addr', ' UNDEF [_]?Py')

###########################################################################
# UNIX
re_allowed_syms = re.compile('|'.join(allowed_unix_symbols))

def unix_shared_lib(config,fname):
    """Tests
    - glibc version
    - no text relocations
    - global symbols
    - undefined symbols"""
    num_ok.count += 1
    errors = []
    glibc = make_pipe( 'eu-readelf -s %s | grep -o -E "@GLIBC_[0-9._]+" | sort -ur | head -n 1' % fname ).strip()
    if glibc != required_glibc:
        errors.append("%s\n%s dependency, should be %s" % (fname,glibc, required_glibc))
    text_reloc = make_pipe('readelf -d %s | grep TEXTREL' % fname, True)
    if text_reloc:
        errors.append("%s\nText relocations found." % fname)
    global_symbols = make_pipe('eu-readelf -s %s | c++filt | grep " GLOBAL " | grep -v "UNDEF"'%fname)
    global_symbols = [i.split()[-1] for i in global_symbols.split('\n') if i]
    for sym in global_symbols:
        if not re_allowed_syms.match(sym):
            errors.append("%s\nForbidden global symbol '%s'"%(fname,sym))
    undef_symbols = make_pipe(' eu-readelf -s %s | c++filt | grep -E " GLOBAL .+UNDEF" | grep -vE "%s"'
                              % (fname, '|'.join(allowed_unix_undef_symbols)), True)
    if undef_symbols:
        errors.append("%s\nForbidden undefined symbols\n%s"%(fname,undef_symbols))
    if errors:
        raise Error('\n'.join(errors))

def process_unix(config, fname):
    dispatch = ( ('*.so*', unix_shared_lib), )
    for pattern, handler in dispatch:
        if os.path.islink(fname):
            pass
        elif fnmatch.fnmatch(fname,pattern):
            handler(config,fname)


###########################################################################
# WINDOWS
class WindowsTools:
    def __init__(self):
        assert not is_unix()
        vs_install_dir = None
        for env in [ 'VS80COMNTOOLS', 'VS90COMNTOOLS', 'VS71COMNTOOLS' ]:
            try:
                vs_install_dir = re.compile( '^%s=(.+)$'%env ).search( make_pipe( 'set '+env ) ).group(1)
                vs_install_dir = vs_install_dir.replace( 'Common7\\Tools\\','' )
                break
            except Error:
                pass
        if not vs_install_dir:
            raise Error( 'Cannot find Visual Studion tools' )
        vs_bin_path = os.path.join( vs_install_dir, 'vc', 'bin' )
        def locate(fname):
            var=os.path.join( vs_bin_path, fname )
            assert os.path.isfile(var)
            return var
        self.dumpbin = locate( 'dumpbin.exe' )
        self.undname = locate( 'undname.exe' )
        self.vcvars = locate( 'vcvars32.bat' )


rex_allowed_dlls = re.compile("|".join(allowed_dlls))
rex_allowed_win_exports = re.compile("|".join(allowed_win_exports))

rex_import_dll = re.compile( '^\s*([a-zA-Z0-9]+\.dll)$', re.I )
rex_export_sym = re.compile( '^\s+[0-9a-fA-F]+ +[0-9a-fA-F]+ +[0-9a-fA-F]+ +([^ ]+)' )
def win_shared_lib(config, arg):
    num_ok.count += 1
    wintool = WindowsTools()
    def check_allowed( text, line_rex, allow_rex, err ):
        for line in text.split('\n'):
            match = line_rex.match(line)
            if match and not allow_rex.match( match.group(1) ):
                raise Error( err % match.group(1))
    try:
        # check imports
        imports = make_pipe('""%s" && "%s" /IMPORTS "%s""' % (wintool.vcvars,wintool.dumpbin,arg))
        check_allowed( imports.lower(), rex_import_dll, rex_allowed_dlls, "Unknown import: %s" )
        # check exports
        tmp=TempFile()
        make_pipe('""%s" && "%s" /EXPORTS "%s"" > %s' % (wintool.vcvars,wintool.dumpbin,arg,tmp.name))
        exports = make_pipe('""%s" %s"' % (wintool.undname,tmp.name))
        check_allowed( exports, rex_export_sym, rex_allowed_win_exports, "Unknown export: %s" )
    except Error, why:
        raise Error( arg + '\n' + why.msg )

def process_win(config, fname):
    dispatch = ( ('*.dll', win_shared_lib),
                 ('*.pyd', win_shared_lib) )
    for pattern, handler in dispatch:
        if fnmatch.fnmatch(fname,pattern):
            handler(config,fname)




###################################
def process(config, arg):
    if is_unix():
        process_unix(config, arg)
    else:
        process_win(config, arg)

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg


def main(argv=None):
    config = {}
    if argv is None:
        argv = sys.argv
    try:
        try:
            opts, args = getopt.getopt(argv[1:], 'ht:', ['help','type='])
        except getopt.error, msg:
            raise Usage(msg)
        # process options
        for o, a in opts:
            if o in ('-h', '--help'):
                print __doc__
                sys.exit(0)
            elif o in ('-t', '--type'):
                assert a=='py' or a=='cpp'
                config['type']=a
        # process arguments
        num_ok.count = 0
#         args = ['/home/jarda/code/jagbase/distribution/bin/libjagpdf.so.1.0.0']
#        args = ['i:/code/jagbase/distribution/bin/jagpdf-1.0.dll']
        for arg in args:
            if os.path.isdir(arg):
                for fname in glob.glob(arg + '/*'):
                    process(config, fname)
            else:
                process(config,arg)
        if not num_ok.count:
            raise Error("No file to check found.")
    except Usage, err:
        print >>sys.stderr, err.msg
        print >>sys.stderr, 'for help use --help'
        return 2
    except Error, err:
        print >>sys.stderr, err.msg
        return 1


if __name__ == '__main__':
    sys.exit(main())

