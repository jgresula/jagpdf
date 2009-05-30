#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import os
import sys
import glob
import md5
import getopt
import re
import heapq
import copy
import fnmatch
import string

if sys.version_info[0]==2 and sys.version_info[1]<4:
    from sets import Set as set

class RetCode:
    def __init__(self):
        self.c = 0

    def set(self, val):
        self.c = val

    def get(self):
        return self.c


def validate_dir(dirname):
    if not os.path.isdir(dirname):
        raise RuntimeError("Directory %s not found." % dirname)
    dirname = os.path.abspath(dirname)
    if not dirname[-1] in ['\\', '/']:
        dirname += '/'
    return dirname


def verify_file_list(dirname, filelist):
    filelist = os.path.abspath(filelist)
    if not os.path.isfile(filelist):
        raise RuntimeError("Filelist '%s' not found."  % filelist)
    # round up all existing files
    found_files = set()
    for dirpath, dirnames, filenames in os.walk(dirname):
        for fname in filenames:
            if not fname.endswith('.log'):
                found_files.add(os.path.abspath(os.path.join(dirpath, fname)))
    assert filelist in found_files
    found_files.remove(filelist)
    # round up all listed files
    listed_files = set()
    for line in open(filelist):
        line = line.strip()
        if not line or line.startswith('#'): continue
        fname = os.path.abspath(os.path.join(dirname, line))
        listed_files.add(fname)
    # on linux, ignore Windows only files
    if 'linux' in sys.platform:
        assert not fnmatch.filter(found_files, '*-windows-only.pdf')
        listed_files = set([f for f in listed_files if not fnmatch.fnmatch(f, '*-windows-only.pdf')])
    # analyze differences
    diff = listed_files.difference(found_files)
    if diff:
        raise RuntimeError("Required file(s) missing:\n " + "\n ".join(diff))
    diff = found_files.difference(listed_files)
    if diff:
        raise RuntimeError("Additional file(s) found:\n " + "\n ".join(diff))


def find_ranges(str1, str2):
    if len(str1) != len(str2):
        raise RuntimeError("Cannot diff, the data have different size.")
    begin=-1 # -1..range not opened
    ranges=[]
    for i, (lhs,rhs) in enumerate(zip(str1,str2)):
        if lhs != rhs:
            if -1 == begin:
                begin = i
        else:
            if -1 != begin:
                ranges.append([begin, i])
                begin = -1
    if -1 != begin:
        ranges.append([begin, i+1])
    return ranges

def find_ranges_in_files(gooddir, fname):
    stem = os.path.basename(fname)
    goodfname = os.path.join(gooddir, stem)
    for b, e in find_ranges(file(fname,'rb').read(),
                            file(goodfname, 'rb').read()):
        print "%s,%s" % (b,e)


def read_range_file(rng_file):
    ranges = []
    if not os.path.isfile(rng_file):
        return ranges
    for line in file(rng_file):
        line = line.strip()
        if line:
            pair = line.split(',')
            ranges.append([int(pair[0]), int(pair[1])])
    return ranges


def find_re_ranges(str1, str2, regexs):
    rex = re.compile('|'.join(regexs), re.M)
    matches1 = [m for m in rex.finditer(str1)]
    matches2 = [m for m in rex.finditer(str2)]
    return [[m1.start(), m1.end()] for m1, m2 in zip(matches1, matches2) if m1.span() == m2.span()]

def find_re_ranges_for_txt(txt, regexs):
    rex = re.compile('|'.join(regexs), re.M)
#     for i, m1 in enumerate(rex.finditer(txt)):
#         print i, m1.group(0)
    return [[m1.start(), m1.end()] for m1 in rex.finditer(txt)]


def strip_ranges(str_, ranges):
    if not ranges:
        return str_
    flattened = [0] + [x for sublist in ranges for x in sublist] + [sys.maxint]
    return ''.join([str_[flattened[i]:flattened[i+1]]
                     for i in range(0, len(flattened), 2)])


def merge_ranges(*ranges):
    all_merged = []
    for rng in ranges:
        all_merged += rng
    if not all_merged:
        return []
    all_merged.sort()
    ranges=[all_merged[0]]
    for rng in all_merged[1:]:
        if rng[0] <= ranges[-1][1]:  #?extends
            if rng[1] > ranges[-1][1]:
                ranges[-1][1] = rng[1]
        else:
            ranges.append(rng)
    return ranges


def md5_on_ranges(str_, ranges):
    m = md5.new()
    m.update(strip_ranges(str_, ranges))
    return m.digest()


def cmp_on_ranges(lhs, rhs, ranges):
    if len(lhs) != len(rhs):
        return False
#     if md5_on_ranges(lhs, ranges) != md5_on_ranges(rhs, ranges):
#         open('/tmp/lhs.txt', 'wb').write(strip_ranges(lhs,ranges))
#         open('/tmp/rhs.txt', 'wb').write(strip_ranges(rhs,ranges))
    return md5_on_ranges(lhs, ranges) == md5_on_ranges(rhs, ranges)


# this function was refactored to be able to compare files with
# different file size, but at the moment this is not possible The
# problem is that the offset to the object table is changed as well
# (not speaking about the offsets in the table)

class Xlator(dict):
    def _make_regex(self):
        return re.compile("|".join(map(re.escape, self.keys())))

    def __call__(self, match):
        return self[match.group(0)]

    def xlat(self, text):
        return self._make_regex().sub(self, text)


def compare_files(lhs, rhs, ranges=[]):
    pdfstr = '(.+?(?<![^\\\]\\\))'
    patterns = [\
        # subset font prefixes
        "/BaseFont/[A-Z]{6}\+",
        "/FontName/[A-Z]{6}\+",
        # creation date
        "/CreationDate \($pdfstr\)",
        # document id
        "/ID\[<[a-f0-9]{32}> <[a-f0-9]{32}>\]",
        # document producer
        "Producer \($pdfstr\)"]
    lhs_data = open(lhs).read()
    rhs_data = open(rhs).read()
    # make sure that ranges are not used, when file sizes differ
    assert not ranges or (len(lhs_data) == len(rhs_data))

    #string.Template supported since 2.4, using Xlator instead
    #patterns = [string.Template(p).substitute(locals()) for p in patterns]
    adict = {'$pdfstr': pdfstr }
    xlat = Xlator(adict)
    patterns = [xlat.xlat(p) for p in patterns]

    def calc_md5(data):
        m = md5.new()
        m.update(data)
        return m.digest()
    def get_stripped_data(data):
        rngs = find_re_ranges_for_txt(data, patterns)
        rngs = merge_ranges(rngs, ranges)
        return strip_ranges(data, rngs)
    lhs_stripped = get_stripped_data(lhs_data)
    rhs_stripped = get_stripped_data(rhs_data)
    if len(lhs_stripped) != len(rhs_stripped):
        return 'wrong lenght'
    if calc_md5(lhs_stripped) != calc_md5(rhs_stripped):
        return 'wrong md5 sum'
    return ''


def verify(outdir, gooddir, files=None):
    def err(msg):
        print 'FAILED:', msg
        ret_code.set(1)
    ret_code = RetCode()
    files = glob.glob(outdir + '/*.pdf')
    files.sort()
    for i, fname in enumerate(files):
        fstem = os.path.basename(fname)
        goodfname = os.path.join(gooddir, fstem)
        if not os.path.isfile(goodfname):
            continue
        print '[%3d/%3d] %-50s' % (i+1, len(files), fstem),
        ranges = read_range_file(goodfname + '.rng')
        error = compare_files(fname, goodfname, ranges)
        if error:
            err(error)
        else:
            print 'ok'

#         if os.path.getsize(fname) != os.path.getsize(goodfname):
#             err('wrong length')
#         else:
#             ranges = read_range_file(goodfname + '.rng')
#             if not compare_files(fname,goodfname,ranges):
#                 err('wrong md5 sum')
#             else:
#                 print 'ok'
    return ret_code.get()



def test():
    assert [[1,2]] == find_ranges('123', '1a3')
    assert [[1,2],[3,4]] == find_ranges('1234', '1a3b')
    assert [[1,3],[4,5]] == find_ranges('12234', '1aa3b')
    assert [] == find_ranges('12234', '12234')
    rngs = find_ranges('12234', '1aa3b')
    assert '13' == strip_ranges('12234', rngs)
    assert cmp_on_ranges('12234', '1aa3b', rngs)
    assert not cmp_on_ranges('1z2234', '1aa3bs', rngs)
    assert [[1,4],[5,7]] == find_re_ranges('1aaf12u',
                                           '1abz12g', ['a[a-z]+', '2[a-z]+'])
    assert [[0,3], [4,11]] == merge_ranges([[1, 2], [4, 8]], [[0, 1], [2, 3], [5, 6], [8, 11]])
    assert [[1,4]] == merge_ranges([[3,4]], find_re_ranges('aD12b',
                                                            'aD13b', ['D..']))
    assert [[1,4]] == merge_ranges([[1,4]], find_re_ranges('aABCa',
                                                            'aDEFa', ['[A-Z]+']))




helpik="""compare_pdfs.py [-l] outdir goodir
compare_pdfs.py -r file gooddir

-r  prints byte offsets where files differ
-l  consult file list (.outfiles in outdir) and report errors
"""

#############################################################
if __name__ == "__main__":
#     test()
#    sys.exit(0)
    cfg = dict(ranges=None,
                verifylist=None,
                outdir=None,
                gooddir=None)
    try:
        optlist, args = getopt.getopt(sys.argv[1:], "o:g:r:l", ['out-dir=',
                                                                 'good-dir=',
                                                                 'find-ranges=',
                                                                 'verify-list'])
        outdir = args
    except RuntimeError, exc:
        print exc
        sys.exit(2)

    try:
        for opt, val in optlist:
            if opt in ['-r', '--find-ranges']:
                cfg['ranges']=val
                #
            elif opt in ['-l', '--verify-list']:
                cfg['verifylist']=True
            elif opt in ['-o', '--out-dir']:
                cfg['outdir']=val
            elif opt in ['-g', '--good-dir']:
                cfg['gooddir']=val
        gooddir = validate_dir(cfg['gooddir'])
        if cfg['ranges']:
            retcode = find_ranges_in_files(gooddir, cfg['ranges'])
        else:
            retcode = None
            outdir = validate_dir(cfg['outdir'])
            if cfg['verifylist']:
                retcode = verify_file_list(outdir, os.path.join(outdir,'.outfiles'))
            if retcode == None:
                retcode = verify(outdir, gooddir)
    except RuntimeError, exc:
        print exc
        retcode = 1

    if retcode:
        sys.exit(retcode)

