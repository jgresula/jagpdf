#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import re
import sys
import getopt
import os

###########################################################################
# path helpers
def splitall(path):
    """Split a path into all of its parts.

    From: Python Cookbook, Credit: Trent Mick
    """
    allparts = []
    while 1:
        parts = os.path.split(path)
        if parts[0] == path:
            allparts.insert(0, parts[0])
            break
        elif parts[1] == path:
            allparts.insert(0, parts[1])
            break
        else:
            path = parts[0]
            allparts.insert(0, parts[1])
    return allparts


def relativepath(fromdir, tofile):
    """Find relative path from 'fromdir' to 'tofile'.

    An absolute path is returned if 'fromdir' and 'tofile'
    are on different drives. Martin Bless, 2004-03-22.
    """
    f1name = os.path.abspath(tofile)
    if os.path.splitdrive(f1name)[0]:
        hasdrive = True
    else:
        hasdrive = False
    f1basename = os.path.basename(tofile)
    f1dirname = os.path.dirname(f1name)
    f2dirname = os.path.abspath(fromdir)
    f1parts = splitall(f1dirname)
    f2parts = splitall(f2dirname)
    if hasdrive and (f1parts[0].lower() <> f2parts[0].lower()):
        "Return absolute path since we are on different drives."
        return f1name
    while f1parts and f2parts:
        if hasdrive:
            if f1parts[0].lower() <> f2parts[0].lower():
                break
        else:
            if f1parts[0] <> f2parts[0]:
                break
        del f1parts[0]
        del f2parts[0]
    result = ['..' for part in f2parts]
    result.extend(f1parts)
    result.append(f1basename)
    return os.sep.join(result)



###########################################################################
# worker
re_pre = re.compile( "^\[/\s+ifdef\s+([a-zA-Z_]+)\s*]|^\[/\s+endif\s*]" )
def main( cfg ):
    gate_open=[True]
    for i, line in enumerate(cfg['in_stream']):
        m = re_pre.search( line )
        if m:
            if None == m.groups()[0]:
                if len(gate_open) == 1: raise RuntimeError( "Mismatched endif (line %d)." % (i+1) )
                gate_open.pop()
            else:
                gate_open.append( m.groups()[0] in cfg['defines'] )
        else:
            if gate_open[-1]:
                line = adjust_paths( line, cfg['rel_path'] )
                cfg['out_stream'].write( line )
    if len(gate_open) != 1:
        raise RuntimeError( "Missing endif (line %d)." % (i+1) )

re_path = re.compile( "^\s*\[(include|xinclude|import)\s+([^\]]+?)\s*\]" )
def adjust_paths( line, rel_path ):
    if rel_path:
        m = re_path.search( line )
        if m:
            return "[%s %s]\n" % ( m.group(1), os.path.join( rel_path, m.group(2) ) )
    return line




###########################################################################
# config
helpik="""usage: qbk_preproc.py [-i infile] [-o outfile] defines
"""

def print_usage():
    print >> sys.stderr, helpik
    sys.exit(1)


def update_cfg_by_stream( cfg, pre ):
    fname = pre+'_fname'
    if None != cfg[fname]:
        if pre=='in' and not os.path.isfile( cfg[fname] ):
            raise RuntimeError( "%s is not a file" % cfg[fname] )
        cfg[fname] = os.path.abspath( cfg[fname] )
        cfg[pre+'_stream'] = open( cfg[fname], pre=='in' and 'r' or 'w' )


def get_config():
    cfg = dict( rel_path = None,
                in_fname = None,
                out_fname = None,
                in_stream = sys.stdin,
                out_stream = sys.stdout,
                defines = set() )
    try:
        optlist, args = getopt.getopt(sys.argv[1:], 'i:o:')
        cfg['defines'] = set(args)
        for opt, val in optlist:
            if opt.startswith( '-i' ):
                cfg['in_fname'] = val
            if opt.startswith( '-o' ):
                cfg['out_fname'] = val
    except Exception, exc:
        print >> sys.stderr, exc
        print_usage()
    try:
        update_cfg_by_stream( cfg, 'in' )
        update_cfg_by_stream( cfg, 'out' )
        if cfg['in_fname'] and cfg['out_fname']:
            cfg['rel_path']=relativepath( os.path.dirname(cfg['out_fname']), cfg['in_fname'] )
            cfg['rel_path']=os.path.dirname(cfg['rel_path'])
    except RuntimeError, exc:
        print >> sys.stderr, exc
        sys.exit(1)
    return cfg


###########################################################################
if __name__ == "__main__":
    cfg = get_config()
    main( cfg )

