#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# generates documentation and copies it to the distribution directory
# JAGBASE_ROOT must be defined (maybe)

import sys
import os
import fnmatch
import shutil

def cmd_stdout( cmd, indata=None ):
    in_, out = os.popen4( cmd )
    if indata:
        in_.write( indata )
    in_.close()
    outdata = out.read()
    ret = out.close()
    retcode = ret!=None and ret or 0
    return retcode, outdata


def get_root_dir():
    root = os.getenv( 'JAGBASE_ROOT' )
    if not root:
        raise RuntimeError( 'JAGBASE_ROOT not defined.' )
    return os.path.abspath(root)


def glob_iter( dirname, patterns ):
    for dirpath, dirnames, filenames in os.walk(dirname):
        for pat in patterns:
            files = [ os.path.join( dirpath, f ) for f in filenames ]
            for fname in fnmatch.filter(files, pat ):
                yield os.path.abspath( fname )


# copying of the resulting html files is done here
def process_html( lang ):
    bjam_args = "-a doc html doc-lang=%s" % lang
    retcode, outdata = run_bjam_cmd( bjam_args )
    print outdata
    if retcode:
        raise RuntimeError( 75*'-' + '\nbjam failed with %d' % retcode )
    copy_htmldoc_files(lang)



def copy_htmldoc_files(lang):
    html_src_dir = os.path.abspath( os.path.join( get_root_dir(), 'doc/quickbook/html/' ) )
    dist_dir = os.path.abspath( os.path.join( get_root_dir(), 'distribution/doc/%s/html' % lang ) )
    prefix_len = len(html_src_dir) + 1
    for fname in glob_iter(html_src_dir, ['*.png', '*.html', '*.css'] ):
        dest = os.path.join( dist_dir, fname[prefix_len:] )
        target_dir = os.path.dirname( dest )
        if not os.path.isdir( target_dir ):
            os.makedirs( target_dir )
        shutil.copyfile( fname, dest )



# copying of certain formats is carried out by the dist-doc target
def process_self_installing( lang, format ):
    bjam_args = "-a dist-doc %s doc-lang=%s" % ( format, lang )
    retcode, outdata = run_bjam_cmd( bjam_args )
    print outdata
    if retcode:
        raise RuntimeError( 75*'-' + '\nbjam failed with %d' % retcode )


# bjam is run from the quickbook directory
def run_bjam_cmd( args ):
    run_dir = os.path.join( get_root_dir(), 'doc/quickbook' )
    curr_dir = os.getcwd()
    os.chdir( run_dir )
    try:
        cmd = 'bjam ' + args
        retcode, output = cmd_stdout( cmd )
    finally:
        os.chdir( curr_dir )
    return retcode, output


def main():
    lang, format = parse_cmd_line()
    if format in ['ps', 'pdf']:
        process_self_installing( lang, format )
    else:
        globals()['process_'+format]( lang )


helpik="""doc_generator.py <c,cpp,python> <html,pdf,ps>"""
def usage():
    print helpik
    sys.exit(1)


def parse_cmd_line():
    """returns (language, format)"""
    def check_arg( arg, allowed ,feat ):
        if arg not in allowed:
            raise RuntimeError( "Unknown %s: %s" % ( feat, arg ) )
    if len(sys.argv) != 3:
        usage()
    check_arg( sys.argv[1], ['c', 'cpp', 'python' ], 'language' )
    check_arg( sys.argv[2], ['html', 'pdf', 'ps' ], 'format' )
    return sys.argv[1:]


if __name__ == "__main__":
    try:
        main()
    except RuntimeError, exc:
        print exc
        sys.exit(1)
