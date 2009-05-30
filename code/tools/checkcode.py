#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import re
import fnmatch
import re
import sys

class Bunch:
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

# class Musters:
#     def __init__(self):
#         d = {}
#         execfile( os.path.join(os.path.dirname(__file__), 'chgdef.py'), d )
#         self.cfg = d['cfg']
#
#     def info_for_pattern(self, pattern):
#         return self.cfg[pattern]
#
#     def patterns( self ):
#         return self.cfg.keys()
#
#
# g_musters = Musters()
#
#
def file_iter(dirname):
    ignore_dirs = ['.svn', '.git', 'jagbase/external']
    for root, dirs, files in os.walk(dirname):
        abs_dirs = [os.path.join(root, d) for d in dirs]
        for ign in ignore_dirs:
            for d in abs_dirs:
                if ign in d:
                    dirs.remove(os.path.basename(ign))
        for f in files:
            yield os.path.join(root, f)
#
#
# def print_code( code ):
#     print '-v----'
#     print code
#     print '-^----'
#
# def print_info( code, newline=True ):
#     if newline:
#         print '>>', code
#     else:
#         print '>>', code,
#
#
# BAD_HEAD = 1 << 0
# BAD_TAIL = 1 << 1
#
# def on_bad_file(  fpath, fcont, state, info ):
#     print "\n[%s%s] %s" % ( state & BAD_HEAD and 'H' or '-',
#                         state & BAD_TAIL and 'T' or '-',
#                         fpath )
#     if state & BAD_HEAD:
#         m_head = info.head_repl_r.search( fcont )
#         if m_head and m_head.start()==0:
#             print_code( fcont[m_head.start():m_head.end()] )
#             print_info( 'REPLACING the leading block' )
#             head_offset = m_head.end()
#         else:
#             print_code( "\n".join( fcont.split( '\n' )[:8]) )
#             print_info( 'no leading block found, INSERTING before' )
#             head_offset = 0
#     if state & BAD_TAIL:
#         if state & BAD_HEAD:
#             print_info( 'key to continue' )
#             msvcrt.getch()
#         m_tail = info.tail_repl_r.search( fcont )
#         if m_tail and m_tail.end()==len(fcont):
#             print_code( fcont[m_tail.start():m_tail.end()] )
#             print_info( 'REPLACING the trailing block' )
#             tail_offset = m_tail.end()
#         else:
#             print_code( "\n".join( fcont.split( '\n' )[-8:]) )
#             print_info( 'no trailing block found, INSERTING after' )
#             tail_offset = -1
#     print_info( '(p)roceed,(e)dit,(s)kip,(q)uit [P]: ', False)
#     while 1:
#         c = msvcrt.getch()
#         c = ord(c)==13 and 'p' or c
#         if c == 'q':
#             print 'quit\n'
#             sys.exit(1)
#         elif c == 's':
#             print 'skip\n'
#             break
#         elif c == 'e':
#             print 'edit\n'
#             os.system( 'notepad.exe "%s"' % fpath )
#             break
#         elif c == 'p':
#             print 'proceed\n'
#             open( fpath + '.bak$', 'w+t' ).write( fcont )
#             if state & BAD_HEAD:
#                 fcont = info.head_t + fcont[head_offset:]
#             if state & BAD_TAIL:
#                 fcont = fcont[:tail_offset] + info.tail_t
#             open( fpath, 'w+t' ).write( fcont )
#             break
#
#
#
#
# def check_file( fpath, mgr):
#     for pattern in mgr.patterns():
#         if fnmatch.fnmatch(fpath, pattern):
#             info = mgr.info_for_pattern( pattern )
#             fcont = open(fpath).read()
#             state = 0
#             if info.head_r and not info.head_r.search(fcont):
#                 state = state | BAD_HEAD
#             if info.tail_r and not info.tail_r.search(fcont):
#                 state = state | BAD_TAIL
#             if state:
#                 on_bad_file( fpath, fcont, state, info )


def read_svn_types(fname):
    result = {}
    for line in open(fname):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        pattern, settings = line.split('=', 1)
        options = settings.split(';')
        result[pattern.strip()] = [o.strip() for o in options]
    return result


def report_unknown_svn_type(fname, ctx):
    f = os.path.basename(fname)
    for pattern in ctx.svn_types.iterkeys():
        if fnmatch.fnmatch(f, pattern):
            return True
    print 'UNKNOWN_TYPE', fname
    return True

re_ignore_patterns = [".*\.pyc$", ".*~$", "/#.*", ".*\.ic[cm]", ".*\.otf",
                      ".*\.rng", ".*\.svg"]
re_ignore = re.compile("|".join(re_ignore_patterns))
def ignore_files(fname, ctx):
    if re_ignore.search(fname):
        return False
    return True

def ignore_binary_file(fname, ctx):
    bytes = open(fname).read(64)
    s = len([None for b in bytes if ord(b)<128])
    return s > .95 * len(bytes)

def change_svn_property(fname, ctx):
    """prints shell commands that set svn properties"""
    f = os.path.basename(fname)
    for pattern in ctx.svn_types.iterkeys():
        if fnmatch.fnmatch(f, pattern):
            for prop in ctx.svn_types[pattern]:
                if '=' in prop:
                    k, v = prop.split('=')
                else:
                    k, v = prop, 'ON'
                print "svn propset", k, v, fname
    return True

re_license = re.compile("MIT license", re.M)
def  files_without_license(fname, ctx):
    if not re_license.search(open(fname).read()):
        print "%s:1:" % fname
    return True




def main(dirname):
    this_dir = os.path.dirname(__file__)
    svn_types = read_svn_types(os.path.join(this_dir, "subversion_types.txt"))
    filters = [ignore_files,
               ignore_binary_file,
               #report_unknown_svn_type
               files_without_license
               ]
    ctx = Bunch(svn_types=svn_types)
    for f in file_iter(dirname):
        for flt in filters:
            if not flt(f, ctx):
                break


if __name__ == "__main__":
    if len(sys.argv) == 1:
        dirname = '.'
    else:
        dirname = sys.argv[1]
    main(dirname)
    sys.exit(1)

