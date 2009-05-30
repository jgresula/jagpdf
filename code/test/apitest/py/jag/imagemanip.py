# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import sys
import array
import math


def _pack_less_than8(seq, w, h, bits):
    assert bits < 8
    buf = array.array('B')
    shift_init = 8-bits
    shift = shift_init
    curr = 0
    max_val = (1<<bits)-1
    i = 0
    for y in xrange(h):
        for x in xrange(w):
            if shift < 0:
                buf.append(curr)
                shift=shift_init
                curr=0
            curr += (int(seq[i]*max_val+0.5))<<shift
            shift -= bits
            i += 1
        buf.append(curr)
        shift=shift_init
        curr=0
    return buf


def _pack_16(seq):
    seq_len = len(seq)
    result = array.array('H', '\0\0'*seq_len)
    i=0
    while i<seq_len:
        result[i] = int(seq[i]*65535+0.5)
        i=i+1
    if sys.byteorder == 'little':
        result.byteswap()
    return result

def _pack_8(seq):
    seq_len=len(seq)
    result = array.array('B', '\0'*seq_len)
    i=0
    while i<seq_len:
        result[i] = int(seq[i]*255+0.5)
        i+=1
    return result


###########################################################################
# public


# f(x,y) over domain <-1, 1.0> that should produce range <-1,1.0>
InvertedEllipseC = lambda x, y: -(0.9 + x*x + y*y - 1)
LineX =            lambda x, y: -x
LineY =            lambda x, y: -y
Cross =            lambda x, y: min(math.fabs(x), math.fabs(y))
InvertedCross =    lambda x, y: -min(math.fabs(x), math.fabs(y))
Rhomboid =         lambda x, y: -(0.9*(math.fabs(x)+math.fabs(y)))/2


def pack_bits(seq, bits, nr_channels, width, height):
    """Retrieves a buffer with an image with the specified bit-depth

    seq    a generic image consisting of samples in range <0..1>
    width  number of columns
    height number of rows
    bits   number of bits
    """
    assert bits in [1,2,4,8,16]
    assert width*height*nr_channels == len(seq)


    if bits == 16:
        return _pack_16(seq)
    elif bits == 8:
        return _pack_8(seq)
    else:
        return _pack_less_than8(seq, width*nr_channels, height, bits)

    assert not "implemented"



def samples_iter(w, h):
    """An iterator of samples over 2d domain <-1,1> x <-1,1>.
    The samples are retrieved row by row.

    w .. number of samples in the horizontal direction
    h .. number of samples in the vertical direction
    """
    xstep = 2.0/(w-1)
    ystep = 2.0/(h-1)
    curry = -1.0
    for y in range(h):
        currx = -1.0
        for x in range(w):
            yield currx, curry
            currx += xstep
        curry += ystep


def normalize_samples(seq):
    """Transforms samples from <-1.0,1.0> -> <0.0,1.0>."""
    for i in range(len(seq)):
        seq[i] = (seq[i]+1.0) / 2.0
        if seq[i] < 0.0:
            seq[i] = 0.0
        elif seq[i] > 1.0:
            seq[i] = 1.0


def image(fn, w, h):
    """Retrieves a sequence of samples <0,1> as a result of
    applying fn to w x h samples.
    """
    val = []
    for x, y in samples_iter(w, h):
        val.append(fn(x, y))
    normalize_samples(val)
    return tuple(val)


def interleave_channels(*channels):
    """Returns a sequence comprising of interleaved channels"""
    return tuple([i for sublist in zip(*channels) for i in sublist])


class ImagePlacer:
    """Places images on a page.
    """
    def __init__(self, writer, page, x, y, dim, step=5):
        self.w = writer
        self.dim = dim
        self.x = x
        self.y = y
        self.step = step
        self.page = page

    def __call__(self, img, desc, ix, iy):
        x, y = self.xy(ix, iy)
        if desc:
            self.page.text(x, y, desc)
        x += (self.dim - img.width())/2
        y += (self.dim - img.height())/2
        self.page.image(img, x, y)

    def xy(self, ix, iy):
        x = self.x + ix*(self.dim+self.step)
        y = self.y + iy*(self.dim+self.step)
        return x, y


class ImageGrid:
    """Creates a grid of images"""
    def __init__(self, doc, startpos, columns, dims, desc_height, gap):
        self.doc = doc
        self.startpos = startpos
        self.dims = dims
        self.desc_height = desc_height
        self.gap = gap
        self.columns = columns
        self.grid_iter = grid_coords(columns, 80)

    def __call__(self, img, desc = None):
        ix, iy = self.grid_iter.next()
        x = self.startpos[0] + ix*(self.dims[0]+self.gap)
        y = self.startpos[1] + iy*(self.dims[1]+self.gap+self.desc_height)
        page = self.doc.page().canvas()
        if desc:
            page.text(x, y, desc)
        x += (self.dims[0] - img.width())/2
        y += (self.dims[1] - img.height())/2
        page.image(img, x, y+self.desc_height)



def grid_coords(w, h):
    for y in range(h):
        for x in range(w):
            yield x, y


class ImgCache:
    def __init__(self):
        self.cache = {}
        self.attempts = 0
        self.misses = 0

    def img_data(self, image, bpc, nr_channels, w, h):
        key = hash((image, bpc, nr_channels, w, h))
        try:
            self.attempts += 1
            return self.cache[key]
        except KeyError:
            img = pack_bits(image, bpc, nr_channels, w, h)
            self.cache[key]=img
            self.misses += 1
            return img

    def stats(self):
        hits = self.attempts - self.misses
        ratio = 100.0 * hits / self.attempts
        return "items: %d, hit ratio %.2f%%, hits %d, misses %d" % (len(self.cache),
                                                                    ratio,
                                                                    hits,
                                                                    self.misses)


def img_cache():
    return ImgCache()


if __name__ == "__main__":
    pass
#     seq = [0.3, 0.4, 0.9,\
#            0.0, 1.0, 0.5]
#     for bits in [1, 2, 4, 8, 16]:
#         buf = pack_bits(seq, bits, 1, 3, 2)
#         buf.tofile(open('seq-%d.bin'%bits, 'wb'))
