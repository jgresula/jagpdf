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
from string import Template
import sys
import md5
from collections import defaultdict
import math
import random
from StringIO import StringIO
import os

AFM_DIR = '../../external/data/Core14_AFMs/'
TYPEMAN_DIR = '../src/resources/typeman/'

class Bunch:
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

class Face:
    def __init__( self, ctx ):
        self.ctx = ctx
        self.chars = []
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
        self.KernGetter = "NULL"
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
        self.getter_fun = None

    def process_line_( self, s ):
        kwd, left, right, value = s.split(' ')
        assert( kwd == 'KPX' )
        left = glyphlist.glyph_to_unicode_map[left]
        right = glyphlist.glyph_to_unicode_map[right]
        # store the kerning info to ctx.kern_dict,
        # which is (left, right) -> {get_fun: value}
        if not self.getter_fun:
            self.getter_fun = 'kern_' + font_name_to_id(self.face.FontName)
            self.face.KernGetter = self.getter_fun
        self.face.ctx.kern_dict[(left,right)][self.getter_fun] = value

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


def process_afm(instream, ctx):
    """processes single afm file"""
    handlers = []
    face = Face(ctx)
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


def process_afm_dir(dirname, ctx):
    """non-recursively processes diretory of afm files"""
    faces = []
    for fname in glob.glob( dirname + '/*.afm' ):
        faces.append(process_afm(open(fname), ctx))
    return faces

###########################################################################
# header generator

cpp_header_muster="""
"""


def do_cpp_header( faces, outs ):
    ENUMS = ",\n    ".join( [ font_name_to_enum(f.FontName) for f in faces ] )
    header_templ = os.path.join(TYPEMAN_DIR, 't1adobestandardfonts.h.template')
    header_templ = open(header_templ).read()
    outs.write( Template(header_templ).substitute(locals()))


###########################################################################
# cpp generator
cpp_impl_muster="""
"""

kern_getter_templ="""
Int $getter_fun(kern_offsets_t const& krec) {
    return krec.$value_holder;
}
"""


def make_kern_pair_key(left, right):
    return left + (right << 16)

def output_kern_table(templ, ctx, getter_to_index, value_to_index):
    # insertion into the hash table depends on randomizer, so make it
    # deterministic here
    random.seed(0)
    # these 3 primes in combination with table size give ~93% load factor
    hash1_p = 14897
    hash2_p = 10709
    hash3_p = 61981
    hash_table_size = 3491
    num_hash_functions = 3
    num_cells = 1
    h = HFunctionsDivision(hash1_p, hash2_p, hash3_p)
    # these 2 primes in combination with table size give ~62% load factor
    hash1_p = 16069
    hash2_p = 43787
    hash_table_size = 5261
    num_hash_functions = 2
    h = HFunctionsDivision(hash1_p, hash2_p)
    # 2 primes, 2 cells -> 91.7%
    hash1_p = 1984061
    hash2_p = 885931
    num_cells = 2
    h = HFunctionsDivision(hash1_p, hash2_p)
    hash_table_size = 1777
    #
    ch = CuckooHash(hash_table_size, h, num_cells)
    result = []
    min_unicode, max_unicode = sys.maxint, 0
    test_a = []
    values = {} # offset tuple -> its index
    values[(0, 0, 0, 0, 0, 0)] = 0
    for k, v in ctx.kern_dict.iteritems():
        key = make_kern_pair_key(*k)
        min_unicode = min(min_unicode, k[0], k[1])
        max_unicode = max(max_unicode, k[0], k[1])
        value = 8 * [0]
        for getter, val in v.iteritems():
            value[getter_to_index[getter]] = value_to_index[val]
        value = tuple(value)
        try:
            value_i = values[value]
        except KeyError:
            value_i = len(values)
            values[value] = value_i
        ch.insert(key, str(value_i))
        test_a.append((key, str(value_i)))
    result += ch.c_output("{0xffffffff, 0}")
    kerning_table = ",\n    ".join(result)
    num_kerning_offsets = len(values)
    offset_list = [(v, k) for k, v in values.iteritems()]
    offset_list.sort()

    off_tuples = (os for i, os in offset_list)
    off_strings = (", ".join(str(o) for o in off) for off in off_tuples)
    offset_c_values = ("{%s}" % s for s in off_strings)
    kerning_offsets = ",\n    ".join(offset_c_values)
                     
    # test
    test_a.sort()
    kerning_sorted = ",\n   ".join(["{0x%x, %s}" % rec for rec in test_a])
    kern_sorted_len = len(test_a)
    # end test
    return Template(templ).safe_substitute(locals())
    

def output_kern_data(templ, ctx):
    """outputs data needed for pair kerning"""
    getters, values = set(), set()
    for pair, d in ctx.kern_dict.iteritems():
        for g, val in d.iteritems():
            getters.add(g)
            values.add(val)
    getter_to_index = dict([(g, i) for i, g in enumerate(getters)])
    vlist = [(v, i + 1) for i, v in enumerate(values)]
    vlist.append((0, 0))
    vlist.sort(lambda l, r : cmp(l[1], r[1]))
    value_to_index = dict(vlist)
    kern_values = ",\n    ".join((str(v) for v, i in vlist))
    templ = output_kern_table(templ, ctx, getter_to_index, value_to_index)
    # output getter functions (they access offset value for given font)
    kerning_getters = []
    for getter_fun, value_holder_i in getter_to_index.iteritems():
        value_holder = "offset_%d" % value_holder_i
        kerning_getters.append(Template(kern_getter_templ).substitute(locals()))
    kerning_getters = "\n".join(kerning_getters)
    return Template(templ).safe_substitute(locals())


def do_cpp_impl(faces, outs, ctx):
    FACE_PTRS = ",\n    ".join( [ "&"+font_name_to_id(f.FontName) for f in faces ] )
    FACE_DEFS = []
    for face in faces:
        FACE_DEFS.append( do_cpp_impl_face(face) )
    FACE_DEFS = "\n".join( FACE_DEFS )
    impl_templ = os.path.join(TYPEMAN_DIR, 't1adobestandardfonts.cpp.template')
    impl_templ = open(impl_templ).read()
    impl_templ = output_kern_data(impl_templ, ctx)
    outs.write(Template(impl_templ).substitute(locals()))


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
    /* kerning getter */         ${KernGetter},    
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
    return Template(cpp_impl_face_muster).substitute( locals() )

def gen_cpp_jagbase():
    ctx = Bunch(kern_dict=defaultdict(lambda : {}))
    faces = process_afm_dir(AFM_DIR, ctx)
    if faces:
        header_file = os.path.join(TYPEMAN_DIR, 't1adobestandardfonts.h')
        do_cpp_header(faces, open(header_file, 'wb' ))
        impl_file = os.path.join(TYPEMAN_DIR, 't1adobestandardfonts.cpp')
        do_cpp_impl(faces, open(impl_file, 'wb'), ctx)


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


# ---------------------------------------------------------------------------
#  kerning stats
#  
def kern_generator():
    from glyphlist import glyph_to_unicode_map as gmap
    for fontfile in glob.glob('../../external/data/Core14_AFMs/*.afm'):
        for line in open(fontfile):
            if line.startswith('KPX'):
                kpx, left, right, offset = line.split()
                yield fontfile, gmap[left], gmap[right], offset
    
def kern_stats():
    # unique lefts per font
    # avg number of rights per single left
    # % of kern pairs in all pair in lorem ipsum
    kd = defaultdict(lambda : {})
    pairs_total = 0
    pairs_unique = set()
    values_unique = set()
    pairs_per_font = defaultdict(lambda : 0)
    pairs_freq_font = defaultdict(lambda : 0)
    max_unicode = 0
    max_left = 0
    max_right = 0
    min_left = sys.maxint
    min_right = sys.maxint
    max_diff = 0
    glyphs = set()
    max_val, min_val = 0, sys.maxint
    for font, left, right, val in kern_generator():
        kd[font][(left, right)] = val
        pairs_total += 1
        pairs_unique.add((left, right))
        values_unique.add(val)
        max_val = max(max_val, int(val))
        min_val = min(min_val, int(val))
        pairs_per_font[font] += 1
        pairs_freq_font[(left, right)] += 1
        max_unicode = max(max_unicode, left, right)
        max_left = max(max_left, left)
        max_right = max(max_right, right)
        min_left = min(min_left, left)
        min_right = min(min_right, right)
        max_diff = max(max_diff, abs(left - right))
        glyphs.add(left)
        glyphs.add(right)
    # post-proc
    pairs_dist = defaultdict(lambda : 0)
    for v in pairs_freq_font.itervalues():
        pairs_dist[v] += 1 
    # out
    log2_glyphs = defaultdict(lambda : 0)
    for g in glyphs:
        log2_glyphs[math.ceil(math.log(g, 2))] += 1
    print 'total:', pairs_total
    print 'unique pairs:', len(pairs_unique), ', tree depth:', math.log(len(pairs_unique), 2)
    print 'unique glyphs:', len(glyphs)
    print 'unique values:', len(values_unique)
    print 'min val:', min_val, ', max_val:', max_val, ", diff:", (max_val - min_val)
    print 'pairs per font:', ', '.join([str(v) for v in pairs_per_font.itervalues()]) 
    print 'pairs freq in fonts:', ', '.join(['%d: %d' % (k, v) for k, v in pairs_dist.iteritems()])
    print 'bits per glyph:', ', '.join(("%d: %d" % (k, v) for k, v in log2_glyphs.iteritems()))
    print 'max unicode:', max_unicode, ', max left:', max_left, ', max right:', max_right
    print 'min left:', min_left, ', min right:', min_right, ', max diff:', max_diff


class CuckooHash:
    def __init__(self, size, hash_funs, cells=1):
        self.num_items = 0
        self.hash_funs = hash_funs
        self.cells = cells
        #assert size % cells == 0
        self.size = size
        self.table =  cells * size * [None]

    def hash(self, i, key):
        return self.hash_funs(i, key) % self.size

    def cell_pos(self, i, key):
        pos = self.hash(i, key)
        return [self.cells * pos + i for i in range(self.cells)]

    def insert(self, key, value):
        pos = self.cell_pos(0, key)
        item = (key, value)
        for n in xrange(self.num_items + 1):
            for p in pos:
                if None == self.table[p]:
                    self.table[p] = item
                    self.num_items += 1
                    return
            p0 = random.choice(pos)
            item, self.table[p0] = self.table[p0], item
            #
            all_pos = [self.cell_pos(i, item[0]) for i in range(self.hash_funs.size())]
            all_pos.remove(pos)
            empty_slots = [pos for pos in all_pos \
                           if [i for i in pos if None == self.table[i]]]
            if empty_slots:
                pos = empty_slots[0]
            else:
                pos = random.choice(all_pos)
        raise TableFull('not possible to insert %d' % item[0])

    def lookup(self, key):
        for i in range(self.hash_funs.size()):
            pos = self.cell_pos(i, key)
            for p in pos:
                if self.table[p] and self.table[p][0] == key:
                    return self.table[p][1]
        return None

#     def insert(self, key, value):
#         pos = self.hash(0, key)
#         assert self.table[pos] == None or self.table[pos][0] != key
#         item = (key, value)
#         for n in xrange(2 * self.num_items + 1):
#             if None == self.table[pos]:
#                 self.table[pos] = item
#                 self.num_items += 1
#                 return 
#             item, self.table[pos] = self.table[pos], item
#             #
#             hashes = [self.hash(i, item[0]) for i in range(self.hash_funs.size())]
#             hashes.remove(pos)
#             empty_slots = [pos for pos in hashes if None == self.table[pos]]
#             if empty_slots:
#                 pos = empty_slots[0]
#             else:
#                 pos = random.choice(hashes)
#         raise TableFull('not possible to insert %d' % item[0])
# 
#     def lookup(self, key):
#         for i in range(self.hash_funs.size()):
#             pos = self.hash(i, key)
#             if self.table[pos] and self.table[pos][0] == key:
#                 return self.table[pos][1]
#         return None

    def stats(self):
        print '#items:', self.num_items
        print 'load factor:', float(self.num_items) / len(self.table)
    
    def load_factor(self):
        return float(self.num_items) / len(self.table)

    def c_output(self, empty_slot):
        result = []
        for i in range(len(self.table)):
            item = self.table[i]
            if item != None:
                result.append("{0x%08x, %s}" % item)
            else:
                result.append(empty_slot)
        return result
                

class TableFull(Exception):
    def __init__(self, msg):
        Exception.__init__(self, msg)

class HFunctionsDivision:
    def __init__(self, *primes):
        self.primes = primes

    def __call__(self, i, key):
        return key % self.primes[i]

    def size(self):
        return len(self.primes)

    def __str__(self):
        return 'Division: ' + ', '.join((str(p) for p in self.primes))

def HDivisionIter():
    h = HFunctionsDivision(1984061, 885931)
    yield CuckooHash(1777, h, 2), h

#     h = HFunctionsDivision(14897, 10709, 61981)
#     yield CuckooHash(3491, h), h
      #
#     from primes import primes
#     while 1:
#         h = HFunctionsDivision(*random.sample(primes,2))
#         yield CuckooHash(5261, h), h
#     from primes import primes
#     while 1:
#         h = HFunctionsDivision(*random.sample(primes,2))
#         yield CuckooHash(1777, h, 2), h
        


class HUniversal:
    def __init__(self, p, *ab):
        self.p = p
        self.ab = ab

    def __call__(self, i, key):
        a, b = self.ab[i]
        return (a * key + b) % self.p

    def size(self):
        return len(self.ab)

    def __str__(self):
        fns = ", ".join(("(%dk + %d) mod %d" % \
                         (ab[0], ab[1], self.p) for ab in self.ab))
        return "Universal: " + fns

def HUniversalIter():
    p = 5387719
    while 1:
        a1 = random.randint(1, p-1)
        b1 = random.randint(0, p-1)
        a2 = random.randint(1, p-1)
        b2 = random.randint(0, p-1)
        h = HUniversal(p, (a1, b1), (a2, b2))
        yield CuckooHash(5261, h), h
        

import itertools
def eratosthenes():
    '''Yields the sequence of prime numbers via the Sieve of Eratosthenes.'''
    D = {  }  # map each composite integer to its first-found prime factor
    for q in itertools.count(2):     # q gets 2, 3, 4, 5, ... ad infinitum
        p = D.pop(q, None)
        if p is None:
            # q not a key in D, so q is prime, therefore, yield it
            yield q
            # mark q squared as not-prime (with q as first-found prime factor)
            D[q*q] = q
        else:
            # let x <- smallest (N*p)+q which wasn't yet known to be composite
            # we just learned x is composite, with p first-found prime factor,
            # since p is the first-found prime factor of q -- find and mark it
            x = p + q
            while x in D:
                x += p
            D[x] = p

def gen_primes(n):
    print "primes = [ \\"
    for p in eratosthenes():
        if p > n:
            break
        print "%d," % p
    print "]"

def next_prime(n):
    for p in eratosthenes():
        if p > n:
            print n
            break


def construct_hash_table():
    pairs_dict = {}
    min_key, max_key = sys.maxint, 0
    for font, left, right, val in kern_generator():
        pairs_dict.setdefault((left, right), {})[font] = val
        min_key = min(min_key, make_kern_pair_key(left, right))
        max_key = max(max_key, make_kern_pair_key(left, right))
    #
    print "min key:", min_key
    print 'max key:', max_key

    found = False
    best_lf = 0.0
    hiter = HUniversalIter()
    hiter = HDivisionIter()
    for h, funs in hiter:
        try:
            for k, v in pairs_dict.iteritems():
                h.insert(make_kern_pair_key(*k), val)
            h.stats()
            found = True
            break
        except TableFull, exc:
            if h.load_factor() > best_lf:
                print 'Load factor: %.3f' % h.load_factor(), 'for', funs
                best_lf = h.load_factor()
    # verify
    if found:
        for k, v in pairs_dict.iteritems():
            assert val == h.lookup(make_kern_pair_key(*k))
        assert h.lookup(make_kern_pair_key(5000, 5000)) == None
        print 'OK for ' + str(funs)
    else:
        print 'FAILED'
    sys.exit(1)
        


if __name__ == "__main__":
    #encoding_status()
    gen_cpp_jagbase()
    #kern_stats()
    #construct_hash_table()
    #gen_primes(0x20002c) # redirect to primes.py
    #next_prime(0x201d0141) # -> 5387719

    #    faces = process_afm_dir( 'c:/Code/cpp/sandbox/jagbase/code/src/resources/typeman/Core14_AFMs/' )
#    do_cpp_header( faces, sys.stdout )
#    do_cpp_impl( faces, sys.stdout )
#     for face in faces:
#         print '>', font_name_to_id( face.FontName )
#    face = process_afm( open('c:/Code/cpp/sandbox/jagbase/code/src/resources/typeman/Core14_AFMs/Helvetica.afm') )
