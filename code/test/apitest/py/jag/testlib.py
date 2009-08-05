#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import jagpdf
import StringIO
import sys
import platform
import traceback

paperA4 = (8.3*72, 11.7*72)
paperA5 = (5.8*72, 8.3*72)

def is_windows():
    return -1 != platform.platform().lower().find('windows')


class EasyFontBase:
    def __init__(self, spec, writer, encoding):
        self.writer = writer
        self.fonts = {}
        self.spec = spec
        if encoding:
            self.spec = spec + ";enc=" + encoding

    def set_writer(self, writer):
        self.writer = writer
        self.fonts = {}

    def __call__(self, ptsize = 12):
        assert(self.writer)
        try:
            fontid = self.fonts[ptsize]
        except KeyError:
            fspec = os.path.expandvars(self.spec % ptsize)
            fontid = self.writer.font_load(fspec)
            self.fonts[ptsize] = fontid
        return fontid

#
class EasyFont(EasyFontBase):
    def __init__(self, writer=None, encoding=None):
        EasyFontBase.__init__(self, 'standard;name=Helvetica;size=%d', writer, encoding)
#
class EasyFontTTF(EasyFontBase):
    def __init__(self, writer=None, encoding=None):
        EasyFontBase.__init__(self, 'size=%d;file=${JAG_TEST_RESOURCES_DIR}/fonts/DejaVuSans.ttf', writer, encoding)

#
#
#
def must_throw(action, *args, **kwds):
    try:
        action(*args, **kwds)
        assert not "Action was expected to throw an exception."
    except jagpdf.Exception:
#    except RuntimeError:
        pass
#     except Exception, exc:
#         print '>>>', exc
#         traceback.print_exc()
#         sys.exit(1)


def must_throw_ex(exc_substring, action, *args, **kwds):
    try:
        action(*args, **kwds)
        assert not "Action was expected to throw an exception."
    except jagpdf.Exception, exc:
#    except RuntimeError, exc:
        if not isinstance(exc_substring, (list,tuple)):
            exc_substring = [exc_substring]
        for substr in exc_substring:
            if not substr in str(exc):
                print '>> substr:', substr
                print str(exc)
                assert not "substring string not found in exception"



class TemporaryFiles:
    def __init__(self):
        self.files = []

    def add(self, filename):
        self.files.append(filename)

    def release(self):
        for fname in self.files:
            if os.path.isfile(fname):
                os.unlink(fname)


#
# Directors
#
# It is important that a class derived from a director calls base
# class constructor.
#

class FileStreamOut(jagpdf.StreamOut):
    def __init__(self, fname):
        jagpdf.StreamOut.__init__(self)
        self.file = open(fname, "wb")

    def write(self, data):
        assert self.file
        self.file.write(data)

    def close(self):
        assert self.file
        self.file.close()
        self.file = None

    def __del__(self):
        super(FileStreamOut,self).__del__()
        if self.file:
            self.close()


class NoopStreamOut(jagpdf.StreamOut):
    def __init__(self):
        jagpdf.StreamOut.__init__(self)
        self.nr_writes = 0
        self.data = StringIO.StringIO()

    def write(self, data):
        self.nr_writes += 1
        self.data.write(data)

    def close(self):
        pass

    def show_stats(self):
        data_len = len(self.data.getvalue())
        print "#writes:     ", self.nr_writes
        print "data len:    ", data_len
        print "avg on write: %.2f bytes" % (self.data_len/float(self.nr_writes))

    def content(self):
        return self.data.getvalue()


def test_config():
    """returns a standard test configuration"""
    cfg = jagpdf.create_profile()
    cfg.set("doc.compressed", "0")
    return cfg


def create_test_doc(argv, fname, cfg=None):
    if None == argv:
        argv = sys.argv
    out_file = os.path.abspath(os.path.join(argv[1], fname))
    cfg = cfg or test_config()
    return jagpdf.create_file(out_file, cfg)


def get_legacy_doc(argv, name, cfg_=None):
    if None==argv:
        argv=sys.argv
    if not cfg_ or isinstance(cfg_,dict):
        cfg = jagpdf.create_profile()
        cfg.set('doc.compressed', "0")
    else:
        cfg = cfg_

    if isinstance(cfg_,dict):
        for opt, val in cfg_.iteritems():
            if isinstance(val, int):
                cfg.set(opt, str(val))
            else:
                cfg.set(opt, val)
    return jagpdf.create_file(os.path.join(argv[1], name), cfg), cfg



def do_background(page, color, dim, margin = 5):
    """Creates a page with background color and margin
    page ... content stream writer
    color ... rgb tuple
    dim  ... page dimensions
    margin ... margin to be left unpainted
    """
    page.state_save()
    page.color_space("fs", jagpdf.CS_DEVICE_RGB)
    page.color("fs", *color)
    m2 = 2*margin
    page.rectangle(margin, margin, dim[0]-m2, dim[1]-m2)
    page.path_paint("fs")
    page.state_restore()


def profile_test(func, *args, **kwds):
    import hotshot, hotshot.stats
    prof = hotshot.Profile('stones.prof')
    prof.runcall(func, *args, **kwds)
    prof.close()
    stats = hotshot.stats.load('stones.prof')
    stats.strip_dirs()
    stats.sort_stats('time', 'calls')
    stats.print_stats(20)
    sys.exit(1)

class ExampleFile:
    """Replaces create_file so that examples included in the
    documentation can have nice names"""
    def __init__(self, orig_fn, argv):
        self.fn = orig_fn
        self.argv = argv

    def __call__(self, fname, cfg=None):
        dir, base = os.path.split(fname)
        base = 'jagpdf_doc_' + base
        fname = os.path.join(dir, base)
        fname = os.path.join(sys.argv[1], fname)
        if cfg:
            return self.fn(fname, cfg)
        else:
            return self.fn(fname)

def paint_image(path, doc, x, y, canvas=None):
    img_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/' + path)
    img = doc.image_load_file(img_file)
    if not canvas:
        canvas = doc.page().canvas()
    canvas.image(img, x, y)

class Matrix:
    def __init__(self):
        self.d = [1, 0, 0, 1, 0, 0]

    def data(self):
        return self.d

    def translate(self, tx, ty):
        self.d[4] = tx * self.d[0] + ty * self.d[2] + self.d[4]
        self.d[5] = tx * self.d[1] + ty * self.d[3] + self.d[5]

    def scale(self, sx, sy):
        self.d[0] = sx * self.d[0]
        self.d[1] = sx * self.d[1]
        self.d[2] = sy * self.d[2]
        self.d[3] = sy * self.d[3]

long_text="""How Software Companies Die
By: Orson Scott Card
The environment that nurtures creative programmers kills management and marketing types - and vice versa. Programming is the Great Game. It consumes you, body and soul. When you\'re caught up in it, nothing else matters. When you emerge into daylight, you might well discover that you\'re a hundred pounds overweight, your underwear is older than the average first grader, and judging from the number of pizza boxes lying around, it must be spring already. But you don\'t care, because your program runs, and the code is fast and clever and tight. You won.
You\'re aware that some people think you\'re a nerd. So what? They\'re not players. They\'ve never jousted with Windows or gone hand to hand with DOS. To them C++ is a decent grade, almost a B - not a language. They barely exist. Like soldiers or artists, you don\'t care about the opinions of civilians. You\'re building something intricate and fine. They\'ll never understand it.
BEEKEEPING
Here\'s the secret that every successful software company is based on: You can domesticate programmers the way beekeepers tame bees. You can\'t exactly communicate with them, but you can get them to swarm in one place and when they\'re not looking, you can carry off the honey.
You keep these bees from stinging by paying them money. More money than they know what to do with. But that\'s less than you might think. You see, all these programmers keep hearing their fathers\' voices in their heads saying \"When are you going to join the real world?\" All you have to pay them is enough money that they can answer (also in their heads) \"Geez, Dad, I\'m making more than you.\" On average, this is cheap.
And you get them to stay in the hive by giving them other coders to swarm with. The only person whose praise matters is another programmer. Less-talented programmers will idolize them; evenly matched ones will challenge and goad one another; and if you want to get a good swarm, you make sure that you have at least one certified genius coder that they can all look up to, even if he glances at other people\'s code only long enough to sneer at it.
He\'s a Player, thinks the junior programmer. He looked at my code. That is enough. If a software company provides such a hive, the coders will give up sleep, love, health, and clean laundry, while the company keeps the bulk of the money.
OUT OF CONTROL
Here\'s the problem that ends up killing company after company. All successful software companies had, as their dominant personality, a leader who nurtured programmers. But no company can keep such a leader forever. Either he cashes out, or he brings in management types who end up driving him out, or he changes and becomes a management type himself. One way or another, marketers get control.
But...control of what? Instead of finding assembly lines of productive workers, they quickly discover that their product is produced by utterly unpredictable, uncooperative, disobedient, and worst of all, unattractive people who resist all attempts at management. Put them on a time clock, dress them in suits, and they become sullen and start sabotaging the product. Worst of all, you can sense that they are making fun of you with every word they say.
SMOKED OUT
The shock is greater for the coder, though. He suddenly finds that alien creatures control his life. Meetings, Schedules, Reports. And now someone demands that he PLAN all his programming and then stick to the plan, never improving, never tweaking, and never, never touching some other team\'s code. The lousy young programmer who once worshipped him is now his tyrannical boss, a position he got because he played golf with some sphincter in a suit.
The hive has been ruined. The best coders leave. And the marketers, comfortable now because they\'re surrounded by power neckties and they have things under control, are baffled that each new iteration of their software loses market share as the code bloats and the bugs proliferate. Got to get some better packaging. Yeah, that\'s it."""

long_unicode_text=u"""V \u00fater\u00fd televize Nova ozn\u00e1mila \u0161irokou ve\u0159ejnost\u00ed dlouho o\u010dek\u00e1vanou zpr\u00e1vu: Tuzemsk\u00e9 poh\u0159ebnictv\u00ed roz\u0161\u00ed\u0159ilo nab\u00eddku slu\u017eeb o odklad rozkladn\u00fdch proces\u016f, lidov\u011b \u0159e\u010deno o balzamov\u00e1n\u00ed mrtvol.
Ji\u017e dlouho mne tr\u00e1p\u00ed ot\u00e1zka vlastn\u00ed nesmrtelnosti, ale dosud se mi nepoda\u0159ilo, aby se po mn\u011b jmenoval alespo\u0148 mal\u00fd brouk. A v podobn\u00e9 situaci jist\u011b je i cel\u00e1 \u0159ada P.T. \u010dten\u00e1\u0159\u016f tohoto sloupku  nic se po nich nejmenuje a nesmrtelnost se nemus\u00ed dostavit.
Nov\u011b zp\u0159\u00edstupn\u011bn\u00e1 slu\u017eba d\u00e1v\u00e1 na\u0161emu byt\u00ed \u00fapln\u011b novou dimenzi. A\u017e dosud se \u010dlov\u011bk musel sm\u00ed\u0159it bu\u010f s organick\u00fdm rozpadem (v p\u0159\u00edpad\u011b poh\u0159bu do zem\u011b), nebo s n\u011bkolik hodin trvaj\u00edc\u00ed teplotou n\u011bkolika set stup\u0148\u016f Celsia (v p\u0159\u00edpad\u011b poh\u0159bu \u017eehem). Nyn\u00ed se nask\u00fdt\u00e1 mnohem p\u0159\u00edjemn\u011bj\u0161\u00ed mo\u017enost  nebo\u017et\u00edkovi v poh\u0159ebn\u00ed slu\u017eb\u011b napumpuj\u00ed do \u017eil formaldehyd, l\u00e1tku, ji\u017e zn\u00e1me z p\u0159\u00edrodov\u011bdn\u00fdch kabinet\u016f  je to ona tekutina, ve kter\u00e9 b\u00fdvaj\u00ed nalo\u017een\u00e9 \u017e\u00e1by. Formal\u00edn pak v t\u011ble nakonzervuje m\u011bkk\u00e9 tk\u00e1n\u011b podobn\u011b, jako to zn\u00e1me z on\u011bch kabinet\u016f, a t\u011blo se st\u00e1v\u00e1 prepar\u00e1tem, ne-li dokonce rovnou mumi\u00ed. Tak\u017ee nesmrtelnost je na dosah.
Jedin\u011b mne trochu mrz\u00ed, \u017ee na mou preparaci nebude m\u00edt kdo dohl\u00e9dnout. Nem\u00e1m potomk\u016f ni man\u017eelky. A tak mus\u00edm otev\u0159en\u011b p\u0159iznat, \u017ee byste to m\u011bli b\u00fdt pr\u00e1v\u011b Vy, moji \u010dten\u00e1\u0159i, kdo zalo\u017e\u00ed fond za vycp\u00e1n\u00ed J.X. Dole\u017eala a posmrtn\u011b mne nech\u00e1 na v\u011b\u010dnou pam\u011b\u0165 vypreparovat i s jointem.
P.S.: Nab\u00eddky na preparaci p\u0159edsmrtnou se s d\u00edky odm\u00edtaj\u00ed."""
