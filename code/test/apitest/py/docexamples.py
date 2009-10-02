# -*- coding: utf-8

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import sys
import jag.testlib as testlib

# A4 = 210x297 mm
#      8.3x11.7 inch
#      597.6x11.79 in 72 DPI

paperA4 = 597.6, 848.68
topleftText = 50, 800

def long_text():
    return """How Software Companies Die
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

master_foo="""Master Foo and the Methodologist
When Master Foo and his student Nubi journeyed among the sacred sites, it was the Master's custom in the evenings to offer public instruction to Unix neophytes of the towns and villages in which they stopped for the night.
On one such occasion, a methodologist was among those who gathered to listen.
\"If you do not repeatedly profile your code for hot spots while tuning, you will be like a fisherman who casts his net in an empty lake,\" said Master Foo.
\"Is it not, then, also true,\" said the methodology consultant, \"that if you do not continually measure your productivity while managing resources, you will be like a fisherman who casts his net in an empty lake?\"
\"I once came upon a fisherman who just at that moment let his net fall in the lake on which his boat was floating,\" said Master Foo. \"He scrabbled around in the bottom of his boat for quite a while looking for it.\"
\"But,\" said the methodologist, \"if he had dropped his net in the lake, why was he looking in the boat?\"
\"Because he could not swim,\" replied Master Foo.
Upon hearing this, the methodologist was enlightened."""

corporate_wisdom_1="""1. A novice asked the master: ``In the east there is a great tree-structure that men call `Corporate Headquarters\'. It is bloated out of shape with vice presidents and accountants. It issues a multitude of memos, each saying `Go, Hence!\' or `Go, Hither!\' and nobody knows what is meant. Every year new names are put onto the branches, but all to no avail. How can such an unnatural entity be?\"
The master replied: ``You perceive this immense structure and are disturbed that it has no rational purpose. Can you not take amusement from its endless gyrations? Do you not enjoy the untroubled ease of programming beneath its sheltering branches? Why are you bothered by its uselessness?\'\'"""

corporate_wisdom_2="""2. In the east there is a shark which is larger than all other fish. It changes into a bird whose wings are like clouds filling the sky. When this bird moves across the land, it brings a message from Corporate Headquarters. This message it drops into the midst of the programmers, like a seagull making its mark upon the beach. Then the bird mounts on the wind and, with the blue sky at its back, returns home.
The novice programmer stares in wonder at the bird, for he understands it not. The average programmer dreads the coming of the bird, for he fears its message. The master programmer continues to work at his terminal, for he does not know that the bird has come and gone."""

class OriginalJagPDF:
    def __init__(self):
        import jagpdf
        self.create_file = jagpdf.create_file

g_jagpdf_orig = OriginalJagPDF()


# ----------------------------------------------------------------------
#                        Profiles
#
def profile_examples(argv):
    import jagpdf
    filename = pdf_file = os.devnull
    my_stream = testlib.NoopStreamOut()

    #---------------------
    #[example_profile_creation
    """` There are several ways how to create a profile object. Here
    is an example of how to retrieve a default profile:"""
    profile = jagpdf.create_profile()
    #` This example will load a profile from a file:
    profile = jagpdf.create_profile_from_file(filename)
    #` And here we will load it from a string:
    profile = jagpdf.create_profile_from_string('doc.compressed=0\n'
                                                'info.author=Your Name')
    #]

    #---------------------
    #[example_profile_usage
    """` Once we have created a profile we can modify it or save it to a file:"""
    profile.set('doc.compressed', '0')
    profile.save_to_file(filename)
    """` And finally, create a PDF document based on it:"""
    doc = jagpdf.create_stream(my_stream, profile)
    #]



# ------------------------------------------------------------------------
#                    Hello world
#
def hello_world(argv):
    """!!! NOTE !!!, when updating the example update also code
    sections below"""
    #[py_example_hello_world
    import jagpdf

    doc = jagpdf.create_file("hello.pdf")
    doc.page_start(597.6, 848.68)
    doc.page().canvas().text(50, 800, "Hello, world!")
    doc.page_end()
    doc.finalize()
    #]
    return
    #[py_example_hello_doccreate
    doc = jagpdf.create_file("hello.pdf")
    #]
    #[py_example_hello_startpage
    doc.page_start(597.6, 848.68)
    #]
    #[py_example_hello_showtext
    doc.page().canvas().text(50, 800, "Hello, world!")
    #]
    #[py_example_hello_finalize
    doc.page_end()
    doc.finalize()
    #]



# ------------------------------------------------------------------------
#                    Error handling
#
def error_handling():
    #[py_example_error_handling
    try:
        #<-
        pass
        #->
        # JagPDF usage
    except jagpdf.Exception, why:
        print why              # formatted error message
    #]

# ------------------------------------------------------------------------
#                    Custom stream
#
def custom_stream():
    import jagpdf
    #[py_example_custom_stream
    class MyStream(jagpdf.StreamOut):
        def write(self, data):
            #<-
            pass
            #->
            # write data
        def close(self):
            #<-
            pass
            #->
            # finish

    doc = jagpdf.create_stream(MyStream())
    #]


# ------------------------------------------------------------------------
#                   class ExampleDocument
#
class ExampleDocument:
    def __init__(self, argv, name, pagew=280, pageh=170):
        import jagpdf
        self.page_dim = [pagew, pageh]
        # jagpdf.create_file actually redirected to testlib.ExampleFile
        self.doc = jagpdf.create_file(name)

    def page_start(self,txt):
        self.doc.page_start(*self.page_dim)
        canvas = self.doc.page().canvas()
        self.doc.outline().item(txt)
        #canvas.text(30, self.page_dim[1]-25, txt)
        return canvas

    def page_end(self):
        self.doc.page_end()

    def finalize(self):
        self.doc.finalize()


# ------------------------------------------------------------------------
#                   class DocumentProxy
#
class DocumentProxy:
    def __init__(self, doc):
        self._doc = doc

    def color_space_load(self, spec):
        icc_dir = os.path.expandvars("profile=${JAG_TEST_RESOURCES_DIR}/icc/")
        spec = spec.replace('profile=', icc_dir)
        return self._doc.color_space_load(spec)

    def image_load_file(self, fname):
        imgdir = os.path.expandvars("${JAG_TEST_RESOURCES_DIR}/images/")
        fname = os.path.join(imgdir, fname)
        return self._doc.image_load_file(fname)

    def font_load(self,fspec):
        fntdir = os.path.expandvars("file=${JAG_TEST_RESOURCES_DIR}/fonts/")
        fspec = fspec.replace('file=', fntdir)
        return self._doc.font_load(fspec)

    def __getattr__(self, name):
        return getattr(self._doc, name)


# ------------------------------------------------------------------------
#                    Paths
#
def paths_examples(argv):
    edoc = ExampleDocument(argv, 'paths.pdf', *paperA4)
    # -------- example page separator ------------
    canvas = edoc.page_start('rectangle')
    #[py_example_line
    canvas.rectangle(50, 400, 500, 350)
    canvas.path_paint('s')
    #]
    edoc.page_end()
    # -------- example page separator ------------
    canvas = edoc.page_start('shape')
    #[py_example_filled_shape
    canvas.move_to(50, 400)
    canvas.line_to(50, 750)
    canvas.line_to(550, 400)
    canvas.line_to(550, 750)
    canvas.path_paint('f')
    #]
    edoc.page_end()
    # -------- example page separator ------------
    edoc.finalize()



# ------------------------------------------------------------------------
#                    Graphics state
#
def graphics_state(argv):
    edoc = ExampleDocument(argv, 'graphics_state.pdf', *paperA4)
    # -------- example page separator ------------
    canvas = edoc.page_start('graphics state')
    #[py_example_save_grstate
    def fill_rectangle(n):
        canvas.rectangle(50, 550+n*60, 500, 55)
        canvas.path_paint('f')

    fill_rectangle(1)
    canvas.state_save()
    canvas.color('f', 0.5)
    fill_rectangle(2)
    canvas.state_restore()
    fill_rectangle(3)
    """` The code draws three rectangles. The first one with a default
    color. Then it saves the graphics state, changes the fill color
    and fills the second rectangle with it. Then the state is
    restored and the last rectangle is filled with the same color as
    the first one."""
    #]
    edoc.page_end()
    edoc.finalize()


# ------------------------------------------------------------------------
#                    Line attributes
#
def line_attributes(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
    page_dim = [597.6, 848.68]
    doc = testlib.create_test_doc(argv, 'line_attributes.pdf', jagpdf.create_profile())
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('dashed lines')
    #[py_example_dashed_line
    """` The following example shows dashed lines (see
    [url_pdfref_chapter Graphics | Graphics State | Details of
    Graphics State Parameters | Line Dash Pattern]):"""
    def dashed_line(n, phase, offset):
        canvas.line_dash(phase, offset)
        canvas.move_to(50, 700+n*15)
        canvas.line_to(550,700+n*15)
        canvas.path_paint('s')

    canvas.line_width(8)
    dashed_line(1, [], 0)
    dashed_line(2, [3], 0)
    dashed_line(3, [2], 1)
    dashed_line(4, [2, 1], 0)
    dashed_line(5, [3, 5], 6)
    dashed_line(6, [2, 3], 11)
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('line cap styles')
    #[py_example_line_cap
    """` The following example shows line cap styles."""
    def stroke_line(n, cap):
        canvas.line_cap(cap)
        canvas.move_to(50, 700+n*30)
        canvas.line_to(550,700+n*30)
        canvas.path_paint('s')

    canvas.line_width(20)
    stroke_line(1, jagpdf.LINE_CAP_BUTT)
    stroke_line(2, jagpdf.LINE_CAP_ROUND)
    stroke_line(3, jagpdf.LINE_CAP_SQUARE)
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('line join styles')
    #[py_example_line_join
    """` The following example shows line join styles."""
    def stroke_shape(n, join):
        canvas.line_join(join)
        canvas.move_to(n*80, 600)
        canvas.line_to(n*80+25, 750)
        canvas.line_to(n*80+50, 600)
        canvas.path_paint('s')

    canvas.line_width(20)
    stroke_shape(1, jagpdf.LINE_JOIN_ROUND)
    stroke_shape(2, jagpdf.LINE_JOIN_BEVEL)
    stroke_shape(3, jagpdf.LINE_JOIN_MITER)
    canvas.line_miter_limit(1.9)
    stroke_shape(4, jagpdf.LINE_JOIN_MITER)
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('join style and opened vs. closed path')
    #[py_example_path_close_join
    """` The following example shows the difference between an opened
    and a closed path in respect to the line join."""
    canvas.line_width(15)
    canvas.line_join(jagpdf.LINE_JOIN_ROUND)
    # opened triangle
    canvas.move_to(50, 600)
    canvas.line_to(100, 750)
    canvas.line_to(150, 600)
    canvas.line_to(50, 600)
    canvas.path_paint('s')
    # closed triangle
    canvas.move_to(200, 600)
    canvas.line_to(250, 750)
    canvas.line_to(300, 600)
    canvas.path_close()
    canvas.path_paint('s')
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()


# ------------------------------------------------------------------------
#                    Clipping path
#
def clipping_path(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
    page_dim = [597.6, 848.68]
    doc = testlib.create_test_doc(argv, 'clipping_path.pdf', jagpdf.create_profile())
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('clipping path')
    #[py_example_clipping_path
    canvas.rectangle(30, 700, 190, 80)
    canvas.path_paint('w')
    canvas.circle(30, 700, 50)
    canvas.path_paint('fs')
    canvas.text(100, 698, 'clipped text')
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('visible clipping path')
    #[py_example_clipping_path_visible
    """` It is possible to combine `'w'` with other operators. In such
    case the path is filled/stroked first and then used as a clipping
    path for the following painting operations."""
    canvas.rectangle(30, 700, 190, 80)
    canvas.path_paint('wfs')
    canvas.color('f', 0.5)
    canvas.color('s', 0.8)
    canvas.circle(125, 740, 60)
    canvas.path_paint('fs')
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()



# ------------------------------------------------------------------------
#                    Color spaces
#
def color_spaces(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
    page_dim = paperA4
    cfg = jagpdf.create_profile()
    #cfg.set("doc.compressed", "0")
    doc = testlib.create_test_doc(argv, 'color_spaces.pdf', cfg)
    doc = DocumentProxy(doc)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('device spaces')
    #[py_example_color_spaces_basic
    canvas.color_space('f', jagpdf.CS_DEVICE_RGB)
    canvas.color_space('s', jagpdf.CS_DEVICE_CMYK)
    canvas.color('f', 1.0, 0.0, 0.0)
    canvas.color('s', 1.0, 0.0, 0.0, 0.0)
    canvas.rectangle(50, 600, 500, 200)
    canvas.path_paint('fs')
    """` Here, DeviceRGB color space and the red color is set for
    stroking and DeviceCMYK and cyan for filling operations."""
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('calrgb')
    #[py_example_color_spaces_lab
    calrgb = doc.color_space_load("calrgb; white=0.9505, 1.089")
    canvas.color_space('f', calrgb)
    canvas.color('f', 0.0, 1.0, 0.0)
    canvas.rectangle(50, 600, 500, 200)
    canvas.path_paint('f')
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('icc')
    #[py_example_color_spaces_icc
    def paint_rectangle(n, color_space):
        cs = doc.color_space_load(color_space)
        canvas.color_space('f', cs)
        canvas.color('f', 0.6, 0.2, 0.7)
        canvas.rectangle(50, 600+n*60, 500, 55)
        canvas.path_paint('f')

    paint_rectangle(1, 'srgb')
    paint_rectangle(2, 'icc; components=3; profile=CIERGB.icc')
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('indexed')
    #[py_example_color_spaces_indexed
    spec = 'srgb; palette=255, 0, 0,'\
                         '0, 255, 0,'\
                         '0, 0, 255'
    cs = doc.color_space_load(spec)
    canvas.color_space('f', cs)
    for i in range(3):
        canvas.color('f', i)
        canvas.rectangle(50, 540+(i+1)*60, 500, 55)
        canvas.path_paint('f')
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()



# ------------------------------------------------------------------------
#                    Color spaces
#
def patterns(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
    page_dim = paperA4
    cfg = jagpdf.create_profile()
    #cfg.set("doc.compressed", "0")
    doc = testlib.create_test_doc(argv, 'patterns.pdf', cfg)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('colored pattern')
    #[py_example_colored_pattern
    pcell = doc.canvas_create()
    pcell.color_space('s', jagpdf.CS_DEVICE_RGB)
    pcell.color('s', 1.0, 0.0, 0.0)
    pcell.circle(15, 15, 15)
    pcell.path_paint('s')
    pattern = doc.tiling_pattern_load("step=30, 30", pcell)
    """` In the code above we created a canvas and painted a red circle on
    it. Then we loaded our pattern and specified the canvas as its pattern
    cell. The option [^step] specifies spacing between pattern cells in both
    directions. Now we can establish the pattern color space, set the pattern as
    the current color and paint a rectangle tiled with our pattern:"""
    canvas.color_space_pattern('f')
    canvas.pattern('f', pattern)
    canvas.rectangle(75, 75, 450, 700)
    canvas.path_paint('f')
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('uncolored pattern')
    #[py_example_uncolored_pattern
    pcell = doc.canvas_create()
    pcell.circle(15, 15, 15)
    pcell.path_paint('s')
    pattern = doc.tiling_pattern_load("step=30, 30", pcell)
    """` Let's establish pattern color space with underlying DeviceRGB color
    space:"""
    canvas.color_space_pattern_uncolored('f', jagpdf.CS_DEVICE_RGB)
    """` Now we will draw two rectangles tiled with our pattern; the pattern
    will be colorized with different colors, green:"""
    canvas.pattern('f', pattern, 0.0, 1.0, 0.0)
    canvas.rectangle(75, 75, 450, 350)
    canvas.path_paint('f')
    """` and blue:"""
    canvas.pattern('f', pattern, 0.0, 0.0, 1.0)
    canvas.rectangle(75, 425, 450, 350)
    canvas.path_paint('f')
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(600, 600)
    canvas = doc.page().canvas()
    label('shading pattern')
    #[py_example_shading_pattern
    """` To illustrate let's show an example of an axial shading pattern. First,
    we will define a Type 2 function which linearly interpolates between red
    and blue:"""
    red_to_blue = doc.function_2_load("domain=0, 1; c0=1, 0, 0; c1=0, 0, 1")
    """` Now we will load our axial shading pattern. The axis will extend from
    (100, 100) to (500, 500):"""
    sh = doc.shading_pattern_load("axial; coords=100, 100, 500, 500",
                                  jagpdf.CS_DEVICE_RGB,
                                  red_to_blue)
    """` Finally, we can establish the pattern color space, set our shading
    pattern as the current color and paint a rectangle:"""
    canvas.color_space_pattern('f')
    canvas.pattern('f', sh)
    canvas.rectangle(0, 0, 600, 600)
    canvas.path_paint('f')
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(600, 600)
    canvas = doc.page().canvas()
    label('shading operator')
    #[py_example_shading_operator
    """` To illustrate, we will load a radial shading pattern defined as two
    concentric circles with center in (0, 0) and radii 0 and 1"""
    sh = doc.shading_pattern_load("radial; coords=0, 0, 0, 0, 0, 1",
                                  jagpdf.CS_DEVICE_RGB, red_to_blue)
    """` Now we will construct a circle that is going to be painted using our
    shading. We will set the circle as the current clipping path:"""
    x, y, radius = 300, 300, 250
    canvas.circle(x, y, radius)
    canvas.path_paint('w')
    """` Before we can finally apply our shading we need to transform the
    current user space to the circle origin and scale it by its radius."""
    canvas.transform(radius, 0, 0, radius, x, y)
    canvas.shading_apply(sh)
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()



# ------------------------------------------------------------------------
#                    Transparency
#
def transparency(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
    page_dim = [597.6, 848.68]
    cfg = jagpdf.create_profile()
    #cfg.set("doc.compressed", "0")
    doc = testlib.create_test_doc(argv, 'transparency.pdf', cfg)
    doc = DocumentProxy(doc)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('alpha')
    #[py_example_alpha
    """` Let's show an alpha constant example:"""
    canvas.line_width(3)
    canvas.color_space('fs', jagpdf.CS_DEVICE_RGB)
    canvas.color('fs', 1.0, 0.0, 0.0)
    canvas.circle(200, 600, 150)
    canvas.path_paint('fs')
    canvas.color('fs', 0.0, 0.0, 1.0)
    canvas.alpha('f', 0.5)
    canvas.circle(350, 600, 150)
    canvas.path_paint('fs')
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()


# ------------------------------------------------------------------------
#                    Coordinate systems
#
def coord_systems(argv):
    import jagpdf
    import math
    def label(txt):
        doc.outline().item(txt)
    page_dim = paperA4
    cfg = jagpdf.create_profile()
    doc = testlib.create_test_doc(argv, 'coord_systems.pdf', cfg)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('coordinate systems')
    #[py_example_transformations
    """` To illustrate coordinate space transformations we will define
    a function which paints a rectangle on the `x`, `y`
    coordinates. We will call it from within variously transformed
    user space to see the effect of the transformation."""
    def paint_rectangle(x, y):
        canvas.rectangle(x, y, 100, 100)
        canvas.path_paint('f')
    """` First, we will paint a rectangle in default user space."""
    paint_rectangle(50, 500)
    """` Now let us translate the origin of user space."""
    canvas.state_save()
    canvas.translate(110, 0)
    paint_rectangle(50, 500)
    canvas.state_restore()
    """` We translated the origin of the user space by `(110, 0)`. The
    rectangle is placed on `(50, 500)` in modified space, i.e. on
    `(160, 500)` in the default user space. We also restored the
    original graphics state so we are back in default user space. The
    next code fragment rotates a rectangle:"""
    x, y = 300, 500
    canvas.state_save()
    canvas.translate(x+50, y+50)
    canvas.rotate(math.pi/4)
    canvas.translate(-(x+50), -(y+50))
    paint_rectangle(x, y)
    canvas.state_restore()
    """` [code_canvas_translate] rotates around the user space
    origin but we want to rotate around the rectangle center. So we
    translated the origin of the user space to the center of the
    rectangle, rotated it there and moved it
    back. [code_canvas_scale] and [code_canvas_skew] behave
    likewise - they scale/skew according to the user space origin."""
    # scale
    x, y = 420, 500
    canvas.state_save()
    canvas.translate(x+50, y+50)
    canvas.scale(0.5, 0.5)
    canvas.translate(-(x+50), -(y+50))
    paint_rectangle(x, y)
    canvas.state_restore()
    # skew
    x, y = 100, 680
    canvas.state_save()
    canvas.translate(x+50, y+50)
    canvas.skew(0.3, 0.3)
    canvas.translate(-(x+50), -(y+50))
    paint_rectangle(x, y)
    canvas.state_restore()
    #]
    doc.page_end()
    doc.finalize()
    # -------- example doc separator ------------
    #[py_example_topdown
    profile = jagpdf.create_profile()
    profile.set('doc.topdown', '1')
    doc = jagpdf.create_file('topdown.pdf', profile)
    # ..
    #]
    doc.page_start(*paperA4)
    fnt = testlib.EasyFont(doc)
    canvas = doc.page().canvas()
    canvas.text_font(fnt(12))
    canvas.text(10, 50, "top-down mode")
    doc.page_end()
    doc.finalize()
    


# ------------------------------------------------------------------------
#                    Images
#
def images(argv):
    import jagpdf
    import math
    import array
    def label(txt):
        doc.outline().item(txt)
    page_dim = [11.7*72, 8.3*72]
    cfg = jagpdf.create_profile()
    doc = testlib.create_test_doc(argv, 'images.pdf', cfg)
    doc = DocumentProxy(doc)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('images')
    #[py_example_image_from_file
    """` The following example shows how to paint an image:"""
    img = doc.image_load_file('logo.png')
    canvas.image(img, 20, 20)
    """` Let's say we would like to tile a region of the page with our
    image. To do so we need to know the image dimensions. Because
    [code_image_width] and [code_image_width] return size in pixels we
    need to recalculate these to user space units."""
    img_width = img.width() / img.dpi_x() * 72
    img_height = img.height() / img.dpi_y() * 72
    for x in range(7):
        for y in range(15):
            canvas.image(img, 90 + x * img_width, 100 + y * img_height)
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('custom image')
    #[py_example_custom_image
    """` In the following example we will show how to paint a custom
    image. First, we will define an 80x80 checker pattern having one
    bit per color."""
    img_data = array.array('B')
    for y in range(80):
        for x in range(10):
            img_data.append((y % 8) > 3 and 0xf0 or 0x0f)
    """` Note that we use [url_py_array] for efficient transfer of
    values between Python and [lib]. We can use an ordinary Python
    list or a tuple as well but [url_py_array] is much faster and
    should be preferred. Now we can define our image:"""
    imgdef = doc.image_definition()
    imgdef.data(img_data)
    imgdef.dimensions(80, 80)
    imgdef.color_space(jagpdf.CS_DEVICE_GRAY)
    imgdef.bits_per_component(1)
    imgdef.dpi(72, 72)
    imgdef.format(jagpdf.IMAGE_FORMAT_NATIVE)
    """` Finally, we load it and paint it:"""
    img = doc.image_load(imgdef)
    canvas.image(img, 50, 450)
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()


# ------------------------------------------------------------------------
#                         Text (Windows only)
#

def text_win32(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
        #canvas.text(30, page_dim[1]-25, txt)
    page_dim = [597.6, 848.68]
    cfg = jagpdf.create_profile()
    doc = testlib.create_test_doc(argv, 'text-windows-only.pdf', cfg)
    doc = DocumentProxy(doc)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Windows - Font Selection')
    #[py_example_font_matching
    verdana = doc.font_load("name=Verdana; size=14; bold; enc=windows-1252")
    canvas.text_font(verdana)
    canvas.text(50, 800, "Verdana Text")
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()



# ------------------------------------------------------------------------
#                          Text - font selection
#
def text_font_selection(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
        #canvas.text(30, page_dim[1]-25, txt)
    page_dim = [597.6, 848.68]
    cfg = jagpdf.create_profile()
    doc = testlib.create_test_doc(argv, 'font_selection.pdf', cfg)
    doc = DocumentProxy(doc)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Font Selection')
    #[py_example_text_simple
    canvas.text(50, 760, "Text")
    #]
    #[py_example_text_simple_courier
    courier = doc.font_load("standard; name=Courier; size=14")
    canvas.text_font(courier)
    canvas.text(50, 780, "Courier Text")
    #]
    #[py_example_text_simple_file
    dejavu = doc.font_load("file=DejaVuSans.ttf; size=14")
    canvas.text_font(dejavu)
    canvas.text(50, 800, "DejaVu Sans")
    #]
    doc.page_end()
    doc.finalize()


# ------------------------------------------------------------------------
#                          Text - encoding
#
def text_encoding(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
    page_dim = [597.6, 848.68]
    cfg = jagpdf.create_profile()
    doc = testlib.create_test_doc(argv, 'font_encoding.pdf', cfg)
    doc = DocumentProxy(doc)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    #[py_example_text_unicode_string
    """` Before we proceed, let's define a unicode variable which we will be
    using in the rest of this section. The human readable form of this string is
    ['žluťoučký kůň úpěl]."""
    unicode_text = u'\u017elu\u0165ou\u010dk\u00fd k\u016f\u0148 \u00fap\u011bl'
    #]
    label('Encoding - Standard Type 1 Fonts')
    #[py_example_core14_latin2
    """`In the following example we will load the standard Helvetica
    font, change its built-in encoding to the ISO-8859-2 encoding and
    set it as the current font:"""
    font = doc.font_load("standard; name=Helvetica; size=14; enc=iso-8859-2")
    canvas.text_font(font)
    """`Now, we can show our ISO-8859-2 encoded text on the page:"""
    text = unicode_text.encode('iso-8859-2')
    canvas.text(50, 800, text)
    #]
    #[py_example_core14_utf8
    """`Here is an example of using Unicode text with a standard font."""
    font = doc.font_load("standard; name=Helvetica; size=14; enc=utf-8")
    canvas.text_font(font)
    canvas.text(50, 780, unicode_text)
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Encoding - TrueType/OpenType')
    #[py_example_text_ttf_enc8
    """`In the following example we will load DejaVuSans.ttf (assuming that it
    resides in the current directory), specify the windows-1250 encoding and set
    it as the current font:"""
    font = doc.font_load("file=DejaVuSans.ttf; size=14; enc=windows-1250")
    canvas.text_font(font)
    """`Now, we can show our windows-1250 encoded text on the page:"""
    text = unicode_text.encode('windows-1250')
    canvas.text(50, 800, text)
    #]
    #[py_example_text_ttf_unicode
    """` We can also specify the UTF-8 encoding:"""
    font = doc.font_load("file=DejaVuSans.ttf; size=14; enc=utf-8")
    canvas.text_font(font)
    text = unicode_text.encode('utf8')
    canvas.text(50, 780, text)
    """` Python unicode objects are automatically converted to UTF-8
    when passed to [lib]'s text showing functions so we can use
    `unicode_text` here as well:"""
    canvas.text(50, 760, unicode_text)
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()



# ------------------------------------------------------------------------
#                          Text - advanced
#
def text_advanced(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
        #canvas.text(30, page_dim[1]-25, txt)
    page_dim = [597.6, 848.68]
    cfg = jagpdf.create_profile()
    doc = testlib.create_test_doc(argv, 'text_advanced.pdf', cfg)
    doc = DocumentProxy(doc)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Advanced Text')
    #[py_example_adv_text_comparison
    canvas.text(50, 800, "text")
    """` It is actually a shortcut for the following sequence:"""
    canvas.text_start(50, 800)
    canvas.text("text")
    canvas.text_end()
    """` Here, we have started a ['text object] with
    [code_canvas_text_start] and moved the origin of ['text space] by
    (50, 800) Then we shown a text string and closed the text object
    with [code_canvas_text_end]."""
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Multiline Text')
    #[py_example_adv_text_multiline
    canvas.text_start(50, 800)
    for n in ['1st', '2nd', '3rd']:
        canvas.text(n)
        canvas.text(' line')
        canvas.text_translate_line(0, -15)
    canvas.text_end()
    """` In this example we have shown three lines of text within a
    text object. We used [code_canvas_text_translate_line] to move to
    the next line."""
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Colored Text')
    #[py_example_adv_text_gr_state
    """` The following example is similar. It shows multi-line text,
    each text is shown in a different color."""
    canvas.text_start(50, 800)
    for perc in range(10, 100, 10):
        canvas.color('f', perc/100.0)
        canvas.text('gray %d%%' % perc)
        canvas.text_translate_line(5, -15)
    canvas.text_end()
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()


# ------------------------------------------------------------------------
#                          Text state
#
def text_state(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
        #canvas.text(30, page_dim[1]-25, txt)
    page_dim = paperA4
    cfg = jagpdf.create_profile()
    doc = testlib.create_test_doc(argv, 'text_state.pdf', cfg)
    doc = DocumentProxy(doc)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Text State')
    canvas.state_save()
    #[py_example_text_charspace
    canvas.text_character_spacing(0.25)
    canvas.text(50, 640, "character spacing")
    #]
    canvas.state_restore()
    canvas.state_save()
    #[py_example_text_scaling
    canvas.text_horizontal_scaling(50)
    canvas.text(50, 660, "horizontal scaling")
    #]
    canvas.state_restore()
    canvas.state_save()
    #[py_example_text_rise
    canvas.text_start(50, 680)
    canvas.text("text rise ")
    canvas.text_rise(5)
    canvas.text("superscripted ")
    canvas.text_rise(-5)
    canvas.text("subscripted")
    canvas.text_end()
    #]
    canvas.state_restore()
    canvas.state_save()
    #[py_example_text_rendering_mode
    font = doc.font_load('standard; name=Helvetica; size=36')
    canvas.text_font(font)
    canvas.color('f', 0.5)
    canvas.text_rendering_mode('f')
    canvas.text(50, 720, "fill text")
    canvas.text_rendering_mode('s')
    canvas.text(50, 760, "stroke text")
    canvas.text_rendering_mode('fs')
    canvas.text(50, 800, "fill and stroke text")
    """` Refer to [code_canvas_text_rendering_mode] to see all
    available rendering modes."""
    #]
    canvas.state_restore()


    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()



# ------------------------------------------------------------------------
#                          Glyph positioning
#
def text_glyph_positioning(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
        #canvas.text(30, page_dim[1]-25, txt)
    page_dim = [597.6, 848.68]
    cfg = jagpdf.create_profile()
    doc = testlib.create_test_doc(argv, 'glyph_positioning.pdf', cfg)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Individual Glyph Positioning')
    #[py_example_text_glyph_pos
    """` To illustrate, let's look at the following example:"""
    canvas.text(50, 750, "AWAY")
    canvas.text(50, 765, "AWAY", [-220, -220, -195], [1, 2, 3])
    """` The first string is shown according to glyph metrics found in
    the current font. In the second case, we passed two lists along
    with the string. The first list specifies offsets, the second one
    their positions in the string. Let's describe what exactly
    happens. First, `A` is painted. Then the text position is moved by
    220 units to the right and `W` is painted. The same is repeated for
    each offset value in the list. So distance between `A` and `W`,
    `W` and `A` and `A` and `Y` is 220, 220 and 195 units
    respectively. In the next example we will change width of space
    between two words to 300 units:"""
    canvas.text(50, 785, "AWAY again")
    canvas.text(50, 800, "AWAY again", [-300], [4])
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()




# ------------------------------------------------------------------------
#                          Font Info
#
def text_font_info(argv):
    import jagpdf
    def label(txt):
        doc.outline().item(txt)
        #canvas.text(30, page_dim[1]-25, txt)
    page_dim = [597.6, 848.68]
    cfg = jagpdf.create_profile()
    doc = testlib.create_test_doc(argv, 'font_info.pdf', cfg)
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Font Info')
    canvas.text_start(50, 800)
    for facename in ['Courier-Oblique', 'Helvetica-Bold', 'Times-Roman']:
        #[py_example_text_font_info_font
        font = doc.font_load('standard; size=14; name=Helvetica')
        #<-
        font = doc.font_load('standard; size=14; name=' + facename)
        #->
        family = font.family_name()
        is_bold = font.is_bold()
        is_italic = font.is_italic()
        size = font.size()
        baseline_distance = font.height()
        ascender = font.ascender()
        descender = font.descender()
        bbox = (font.bbox_xmin(), font.bbox_ymin(), \
                font.bbox_xmax(), font.bbox_ymax())
        #]
        def next_line(indent=0, lspacing=1):
            canvas.text_translate_line(indent, -lspacing*baseline_distance)
        canvas.text_font(font)
        canvas.text(facename)
        next_line(5)
        canvas.text("family: " + family)
        next_line()
        canvas.text("size: %.0f" % size)
        next_line()
        canvas.text("bold: %d" % is_bold)
        next_line()
        canvas.text("italic %d" % is_italic)
        next_line()
        canvas.text("ascender: %.3f" % ascender)
        next_line()
        canvas.text("descender: %.3f" % descender)
        next_line()
        canvas.text("baseline distance: %.3f" % baseline_distance)
        next_line()
        canvas.text("bbox: [%.3f, %.3f, %.3f, %.3f]" % bbox)
        next_line(-5, 2)
    canvas.text_end()
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Metrics')
    #[py_example_text_font_info_metrics
    """` In the next example we will illustrate font metrics visually.
    Let's define a function drawing metrics for a given glyph."""
    def show_glyph_metrics(x, y, font, char):
        #<-
        canvas.color('s', 0.7)
        #->
        advance = font.advance(char)
        # bounding box
        bbox_w = font.bbox_xmax() - font.bbox_xmin()
        bbox_h = font.bbox_ymax() - font.bbox_ymin()
        canvas.rectangle(x+font.bbox_xmin(), y+font.bbox_ymin(), bbox_w, bbox_h)
        # advance
        canvas.move_to(x, y)
        canvas.line_to(x + advance, y)
        # ascender & descender
        canvas.rectangle(x, y + font.descender(), \
                         advance, font.ascender() - font.descender())
        # show it
        canvas.path_paint('s')
        canvas.text(x, y, char)
    """` Note that all numeric values retrieved from [code_font] are
    in default user space units. Now we can load a font and show glyph
    metrics e.g. for `'g'`:"""
    f = doc.font_load('standard; size=60; name=Helvetica')
    canvas.text_font(f)
    show_glyph_metrics(140, 740, f, 'g')
    #]
    y = 740
    for c in "pfWMy;{?'":
        y -= f.height()
        show_glyph_metrics(140, y, f, c)
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    canvas.color('s', 0.5)
    label('Advance')
    #[py_example_text_font_info_advance
    """` Font properties are useful when we want to perform more
    advanced text formatting. In the following example we will show
    how to use font properties to implement various line alignment
    styles. Let's start with defining required line width:"""
    line_width = 558
    #<-
    canvas.rectangle(20, 400, line_width, 115)
    canvas.path_paint('s')
    #->
    """` Now we will load a font and retrieve width of a text string
    which is going to be aligned:"""
    font = doc.font_load('standard; size=16; name=Times-Roman')
    txt = "Everything should be made as simple as possible, but no simpler."
    text_width = font.advance(txt)
    """` So let's align our text to the left - there is nothing
    special about it:"""
    canvas.text_start(20, 420)
    canvas.text_font(font)
    canvas.text(txt)
    canvas.text_translate_line(0, font.height())
    """` Now we will calculate `padding` which represents amount of
    whitespace that should be added to our text string to match
    `line_width`."""
    gap = line_width - text_width
    padding = -1000.0 / font.size() * gap
    """` We can align the string to the right by offsetting the
    first glyph by `padding`:"""
    canvas.text(txt, [padding], [0])
    canvas.text_translate_line(0, font.height())
    """` Or we can align on the center:"""
    canvas.text(txt, [padding / 2.0], [0])
    canvas.text_translate_line(0, font.height())
    """` Finally, a justified text string can be achieved by
    distributing `padding` evenly among spaces:"""
    spaces = [i for i in range(len(txt)) if txt[i] == ' ']
    num_spaces = len(spaces)
    canvas.text(txt, num_spaces * [padding / num_spaces], spaces)
    canvas.text_end()
    #]
    doc.page_end()
    # -------- example page separator ------------
    #[py_example_text_font_formatter_impl
    """` The first thing we need for achieving of our goal is to
    implement a line breaking function which for given paragraph and
    line width finds line breaks and calculates width of individual
    lines."""
    def line_breaking(font, para, width):
        lines = []
        space = font.advance(' ')
        line, curr_width = [], 0
        for word in para.split():
            adv = font.advance(word)
            if curr_width + space + adv > width:
                lines.append((' '.join(line), curr_width - space))
                line, curr_width = [], 0
            curr_width += space + adv
            line.append(word)
        if line:
            lines.append((' '.join(line), curr_width - space))
        return lines
    """` Now we can implement a text formating function. We will find
    line breaks for each paragraph and align it according to
    `alignment`."""
    def format_text(x, y, canvas, font, text, width, alignment):
        canvas.text_font(font)
        canvas.text_start(x, y)
        for para in text.split('\n'):
            lines = line_breaking(font, para, width)
            for line, line_width in lines:
                gap = width - line_width
                padding = -1000.0 / font.size() * gap
                if alignment == 'left':
                    canvas.text(line)
                elif alignment == 'right':
                    canvas.text(line, [padding], [0])
                elif alignment == 'centered':
                    canvas.text(line, [padding / 2.0], [0])
                elif alignment == 'justified':
                    spaces = [i for i in range(len(line)) if line[i] == ' ']
                    num_spaces = len(spaces)
                    if not num_spaces or line == lines[-1][0]:
                        canvas.text(line)
                    else:
                        canvas.text(line, num_spaces * [padding / num_spaces], spaces)
                canvas.text_translate_line(0, -font.height())
            canvas.text_translate_line(0, 1.5 * -font.height())
        canvas.text_end()
    #]
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Justified')
    #[py_example_text_font_formatter_use
    """` Now that we have our text formatter, we can try it
    out. `long_text()` is a function returning a multi-paragraph text string."""
    font = doc.font_load('standard; size=10; name=Helvetica')
    format_text(20, 822, canvas, font, long_text(), 558, "justified")
    #]
    doc.page_end()
    for adj in ['Left', 'Right', 'Centered']:
        doc.page_start(*page_dim)
        canvas = doc.page().canvas()
        label(adj)
        format_text(20, 11.7*72-20, canvas, font, long_text(), 8.3*72-40, adj.lower())
        doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Master Foo')
    format_text(20, 622, canvas, font, master_foo, 558, "centered")
    doc.page_end()
    # -------- example page separator ------------
    doc.page_start(*page_dim)
    canvas = doc.page().canvas()
    label('Corporate Wisdom')
    format_text(20, 722, canvas, font, corporate_wisdom_1, 558, "justified")
    format_text(20, 522, canvas, font, corporate_wisdom_2, 558, "justified")
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()

# ------------------------------------------------------------------------
#                          Destinations
#
def destinations():
    import jagpdf
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    outline = doc.outline()
    doc.page_start(10.0*72, 10.0*72)
    #[py_example_destination_str
    """` Here is an example of a destination definition:"""
    destination_str = "mode=XYZ; left=72; right=144; zoom=2.5"
    #]
    #[py_example_destination_id
    destination_obj = doc.destination_define("mode=Fit")
    #]
    outline.item("T", destination_str)
    doc.page_end()
    doc.finalize()

# ------------------------------------------------------------------------
#                          Document Outline
#
def document_outline(argv):
    import jagpdf
    # -------- example doc separator ------------
    doc = testlib.create_test_doc(argv, 'document_outline_basic.pdf')
    #[py_example_docoutline_basic
    """` [lib] provides class [code_documentoutline] which allows to
    form a document outline. We can retrieve its instance from
    [code_document_outline]:"""
    outline = doc.outline()
    """` So let's create a document with six pages and create an
    outline item for each page:"""
    for i in [1, 2, 3, 4, 5, 6]:
        doc.page_start(597.6, 848.68)
        outline.item('Go to page %d' % i)
        #<-
        doc.page().canvas().text(50, 800, 'Page %d' % i)
        #->
        doc.page_end()
    #]
    doc.finalize()
    # -------- example doc separator ------------
    doc = testlib.create_test_doc(argv, 'document_outline_hierarchy.pdf')
    outline = doc.outline()
    #[py_example_docoutline_hierarchy
    """` In the previous example we have created a flat document
    outline. Let's extend our example and form a hierarchy of outline
    items by with help of [code_documentoutline_level_down] and
    [code_documentoutline_level_up]"""
    for i in [1, 2, 3, 4, 5, 6]:
        doc.page_start(597.6, 848.68)
        #<-
        doc.page().canvas().text(50, 800, "Page %d" % i)
        #->
        if i % 2:
            outline.item('Go to page %d' % i)
        else:
            outline.level_down()
            outline.item('Go to page %d' % i)
            outline.level_up()
        doc.page_end()
    """` Now, we have made even pages children of odd pages."""
    #]
    doc.finalize()
    # -------- example doc separator ------------
    doc = testlib.create_test_doc(argv, 'document_outline_destinations.pdf')
    outline = doc.outline()
    #[py_example_docoutline_destinations
    """` The following example illustrates how to associate a
    destination with an outline item. We will create several outline
    items representing different views (destinations) of a single
    page."""
    doc.page_start(597.6, 848.68)
    #<-
    doc.page().canvas().text(50, 800, 'Beware of programmers who carry screwdrivers.')
    #->
    outline.item('mode XYZ, zoom 300%', 'mode=XYZ; zoom=3.0')
    outline.item('mode Fit', 'mode=Fit')
    outline.item('mode FitH', 'mode=FitH; top=800')
    outline.item('mode FitV', 'mode=FitV; top=0')
    doc.page_end()
    #]
    doc.finalize()
    # -------- example doc separator ------------
    doc = testlib.create_test_doc(argv, 'document_outline_post.pdf')
    outline = doc.outline()
    #[py_example_docoutline_post
    """` In certain cases we might want a document outline not to
    follow the order of pages. For instance, let's create a document
    outline referring to pages in reverse order:"""
    for i in [1, 2, 3]:
        doc.page_start(597.6, 848.68)
        #<-
        doc.page().canvas().text(50, 800, 'Page %d' % i)
        #->
        doc.page_end()
    for i in [2, 1, 0]:
        outline.item('Page %d' % (i+1), 'mode=XYZ; page=%d; top=848.68' % i)
    """` The trick is the `'page'` option in the destination
    definition. It explicitly says that we want the destination on
    that particular page. If we do not specify `'page'`, as we have
    done in our examples so far, then the current page is used
    (assuming we are between [code_document_page_start] and
    [code_document_page_end]). Thus, the `'page'` option allows us to
    create an arbitrary document outline structure regardless of the
    order of pages."""
    #]
    doc.finalize()
    # -------- example doc separator ------------


# ------------------------------------------------------------------------
#                          Link Annotations
#
def annotation_link(argv):
    import jagpdf
    # -------- example doc separator ------------
    doc = testlib.create_test_doc(argv, 'annotation_link_dest.pdf')
    doc.page_start(*paperA4)
    canvas = doc.page().canvas()
    #[py_example_annotation_link_dest
    """` In the following example we will associate a text on the
    first page with a destination representing the second page. Let's
    start with showing a text on the first page:"""
    font = doc.font_load('standard; name=Helvetica; size=24')
    canvas.text_font(font)
    x, y = 50, 800
    text = 'Click this text to go to page 2.'
    canvas.text(x, y, text)
    """` Now, we need to calculate the bounding box of our text:"""
    bbox_width = font.advance(text)
    bbox_height = font.bbox_ymax() - font.bbox_ymin()
    """` And associate a destination with it:"""
    doc.page().annotation_goto(x, y + font.bbox_ymin(),
                               bbox_width, bbox_height,
                               "mode=Fit; page=1")
    #]
    doc.page_end()
    doc.page_start(*paperA4)
    doc.page().canvas().text(x, y, "This is page 2.")
    doc.page_end()
    doc.finalize()
    # -------- example doc separator ------------
    doc = testlib.create_test_doc(argv, 'annotation_link_uri.pdf')
    doc = DocumentProxy(doc)
    doc.page_start(*paperA4)
    canvas = doc.page().canvas()
    #[py_example_annotation_link_uri
    """` The following example illustrates how to associate a URI with
    a location on a page occupied by an image. First, we will paint
    the image:"""
    img = doc.image_load_file('logo.png')
    canvas.image(img, 50, 750)
    """` Retrieve the image dimensions:"""
    width = img.width() / img.dpi_x() * 72
    height =img.height() / img.dpi_y() * 72
    """` And use them to associate a URI with the image:"""
    doc.page().annotation_uri(50, 750, width, height, "http://jagdpf.org")
    #]
    doc.page_end()
    """` """
    doc.finalize()
    # -------- example doc separator ------------
    def text_bbox(y, font, txt):
        width = font.advance(text)
        height = font.bbox_ymax() - font.bbox_ymin()
        return y + font.bbox_ymin(), width, height
    doc = testlib.create_test_doc(argv, 'annotation_link_destfwd.pdf')
    #[py_example_annotation_link_destfwd
    """` The following example demonstrates how to work with reserved
    destinations. Let's paint a rectangle first:"""
    doc.page_start(597.6, 848.68)
    canvas = doc.page().canvas()
    canvas.rectangle(100, 700, 80, 80)
    canvas.path_paint('s')
    """` Associate an empty destination with the rectangle:"""
    dest = doc.destination_reserve()
    doc.page().annotation_goto(100, 700, 80, 80, dest)
    doc.page_end()
    """` Now start the second page and show a text:"""
    doc.page_start(597.6, 848.68)
    doc.page().canvas().text(50, 300, 'Heading')
    """` We ['know] the destination now so we can define it:"""
    doc.destination_define_reserved(dest, "mode=XYZ; top=315")
    doc.page_end()
    #]
    doc.page_start(597.6, 848.68)
    doc.page().canvas().text(50, 800, 'The last page.')
    doc.page_end()
    doc.finalize()
    # -------- example doc separator ------------


# ------------------------------------------------------------------------
#                    Encryption
#
def encryption(argv):
    import jagpdf
    #[py_example_encryption_basic
    """` In the following example we will show, how to encrypt a
    document.  First we will create a profile and specify that we want
    to use the standard security handler:"""
    profile = jagpdf.create_profile()
    profile.set("doc.encryption", "standard")
    #<-
    profile.set("info.static_producer", "1")
    profile.set("info.creation_date", "0")
    #->
    """` Now we will use that profile when a new document is created"""
    doc = jagpdf.create_file("encrypted.pdf", profile)
    doc.page_start(597.6, 848.68)
    doc.page().canvas().text(50, 800, "Encrypted Document.")
    doc.page_end()
    doc.finalize()
    """` A document encrypted in this way has neither user nor owner
    password and there are no restrictions on user operations."""
    #]
    # -------- example doc separator ------------
    #[py_example_encryption_pwd
    """` To create a document with passwords we need to specify them
    in a profile:"""
    profile = jagpdf.create_profile()
    profile.set("doc.encryption", "standard")
    profile.set("stdsh.pwd_owner", "owner-pwd")
    profile.set("stdsh.pwd_user", "user-pwd")
    #<-
    profile.set("info.static_producer", "1")
    profile.set("info.creation_date", "0")
    #->
    """` And initialize the document with that profile:"""
    doc = jagpdf.create_file("encrypted_pwd.pdf", profile)
    doc.page_start(597.6, 848.68)
    doc.page().canvas().text(50, 800, "Document with user and owner passwords.")
    doc.page_end()
    doc.finalize()
    #]
    # -------- example doc separator ------------
    #[py_example_encryption_perm
    """` In the following example we will illustrate how to set user
    permissions."""
    profile = jagpdf.create_profile()
    profile.set("doc.encryption", "standard")
    profile.set("stdsh.permissions", "no_print; no_copy")
    #<-
    profile.set("info.static_producer", "1")
    profile.set("info.creation_date", "0")
    #->
    """` We specified that the user is not allowed to print or extract
    text from the document. And again, we need to supply the profile
    to the document:"""
    doc = jagpdf.create_file("encrypted_perm.pdf", profile)
    doc.page_start(597.6, 848.68)
    doc.page().canvas().text(50, 800, "No printing or text copying allowed")
    doc.page_end()
    doc.finalize()
    #]
    # -------- example doc separator ------------


# ------------------------------------------------------------------------
#                    Documentation conventions
#
def doc_conventions():
    #[py_documentation_conventions
    """` Almost all code fragments in this document assume the following
    context:"""
    import jagpdf

    #<-
    def fake_create(n):
        return jagpdf.create_stream(testlib.NoopStreamOut())
    create_file_orig = jagpdf.create_file
    jagpdf.create_file = fake_create
    #->
    doc = jagpdf.create_file("example.pdf")
    doc.page_start(597.6, 848.68)
    canvas = doc.page().canvas()

    #
    # Here comes the code fragment.
    #

    doc.page_end()
    doc.finalize()
    #]
    # restore the function
    jagpdf.create_file = create_file_orig


# ------------------------------------------------------------------------
#                             Functions
#
def functions():
    import jagpdf
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    doc.page_start(200, 200)
    #[py_functions_type2
    """` Type 2 functions are 1-in, n-out functions that perform exponential
    interpolation. The following example shows how to load a Type 2 function:"""
    red_to_green = doc.function_2_load("domain=0, 1; c0=1, 0, 0; c1=0, 1, 0")
    green_to_blue = doc.function_2_load("domain=0, 1; c0=0, 1, 0; c1=0, 0, 1")
    """` These are examples of 1-in, 3-out functions which interpolate linearly
    between `c0` and `c1` defined over (0, 1) domain. For instance, if we
    interpret `c0` and `c1` as RGB color values then the functions above
    represent linear interpolation between red and green (`red_to_green`) and
    green and blue (`green_to_blue`)."""
    #]
    #[Py_functions_type3
    """` Type 3 functions combine several 1-input functions into a single new
    1-input function. Let's show how to combine `red_to_green` and
    `green_to_blue` functions from the previous example:"""
    fn3 = doc.function_3_load("domain=0, 1; bounds=0.5; encode=0, 1, 0, 1",
                              [red_to_green, green_to_blue])
    """` The `red_to_green` function applies to input values in <`domain[0]`,
    `bounds`), the `green_to_blue` function applies to input values in
    <`bounds`, `domain[1]`). The pairs in the ['encode] array map <`domain[0]`,
    `bounds`) and <`bounds`, `domain[1]`) to domains of the corresponding
    functions."""
    #]
    #[py_functions_type4
    """` Type 4 functions are defined by a subset of the PostScript
    language. The input values are pushed to the initial operand stack. After
    the execution of the function the operand stack contains the output
    values. The number of operands left on the operand stack must be in concert
    with the `range` option. To illustrate, let's load a 2-in, 1-out function:"""
    rhomboid = doc.function_4_load("domain=-1, 1, -1, 1; range=-1, 1;"
                                   "func={abs exch abs 0.9 mul add 2 div}")
    """` The stack will initially contain two input variables and after
    executing the function there will be a single output variable."""
    #]
    doc.page_end()
    # -------- example page separator ------------
    doc.finalize()


# ------------------------------------------------------------------------
#                    Strings
#
def strings(argv):
    # ticket regarding https://svn.boost.org/trac/boost/ticket/2860
    #https://svn.boost.org/trac/boost/ticket/2860
    import jagpdf
    doc = testlib.create_test_doc(argv, 'py_strings.pdf')
    doc.page_start(*paperA4)
    doc = DocumentProxy(doc)
    canvas = doc.page().canvas()
    #[py_example_string
    """`To illustrate, let's load a standard ISO-8859-2 encoded font."""
    helv = doc.font_load("standard; enc=iso-8859-2; name=Helvetica; size=24")
    canvas.text_font(helv)
    """` One of the languages representable by this encoding is Czech. Now that
    we have loaded the font, we can pass an ISO-8859-2 encoded string ['úplněk]
    (full moon). There are two equivalent ways to do so: by encoding a `unicode`
    string to ISO-8859 or using a ISO-8859-2 encoded string literal."""
    full_moon_cze = u"\u00fapln\u011bk"
    canvas.text(50, 800, full_moon_cze.encode('iso-8859-2'))
    canvas.text(50, 760, "\xfapln\xeck")
    """` [lib] converts `unicode` objects to UTF-8 so the following will not be
    shown correctly because ISO-8859-2 encoded string is expected."""
    canvas.text(50, 720, full_moon_cze) # wrong
    """`Python correctly raises ['UnicodeEncodeError] when we attempt to encode
    Swedish ['fullmåne] to ISO-8859-2 as ['å] cannot be represented by this
    encoding. We must use ISO-8859-1 encoded font instead."""
    full_moon_swe = u"fullm\u00e5ne"
    try:
        canvas.text(50, 680, full_moon_swe.encode('iso-8859-2'))
    except UnicodeEncodeError, exc:
        print exc
    #` Let's load a Unicode encoded TrueType font.
    dejavu = doc.font_load("enc=utf-8; file=DejaVuSans.ttf; size=24");
    canvas.text_font(dejavu);
    """` Now we can mix Czech and Swedish. We can pass either a `unicode` object
    or a UTF-8 encoded `str` object."""
    canvas.text(50, 680, full_moon_cze)
    canvas.text(50, 640, full_moon_swe)
    canvas.text(50, 600, full_moon_swe.encode('utf-8'))
    """` Note that in the following case `'full moon'` is a UTF-8 encoded `str`
    object as all used characters are in the range 0x00-0x7f."""
    canvas.text(50, 560, "full moon") # ok
    #]
    doc.page_end()
    doc.finalize()

# ------------------------------------------------------------------------
#                          Glyph Indices
#
def glyph_indices(argv):
    import jagpdf
    doc = testlib.create_test_doc(argv, 'glyph_indices.pdf')
    doc = DocumentProxy(doc)
    doc.page_start(*paperA4)
    canvas = doc.page().canvas()
    #[py_example_glyph_indices
    dejavu = doc.font_load("file=DejaVuSans.ttf; size=14")
    canvas.text_font(dejavu)
    # glyph indices for 'g', 'l', 'y', 'p', 'h', and 's'
    glyphs = [74, 79, 92, 83, 75, 86]
    canvas.text_glyphs(50, 800, glyphs)
    canvas.text_glyphs(50, 780, glyphs, [-130.0, -130.0], [2, 3])
    """` Note that the second [^text_glyphs()] individually positions the 2nd and
    3rd glyph. We can also retrieve width of the glyph at a given index:"""
    # width of the glyph at index 74, i.e. 'g'
    dejavu.glyph_width(74)
    #]
    doc.page_end()
    doc.finalize()
    


# ------------------------------------------------------------------------
#                    Main
#

def test_main(argv=None):
    if argv==None:
        argv=sys.argv

    # importing jagpdf into a local scope does not affect jagpd.create_file
    import jagpdf
    jagpdf.create_file = testlib.ExampleFile(jagpdf.create_file, argv)

    hello_world(argv)
    profile_examples(argv)

    error_handling()
    custom_stream()

    paths_examples(argv)
    graphics_state(argv)
    line_attributes(argv)
    clipping_path(argv)
    color_spaces(argv)
    patterns(argv)
    transparency(argv)
    coord_systems(argv)
    images(argv)
    text_font_selection(argv)
    text_encoding(argv)
    text_advanced(argv)
    text_state(argv)
    text_font_info(argv)
    text_glyph_positioning(argv)
    destinations()
    document_outline(argv)
    annotation_link(argv)
    encryption(argv)
    doc_conventions()
    functions()
    glyph_indices(argv)
    strings(argv)
    if 'win32' in sys.platform:
        text_win32(argv)

if __name__ == '__main__':
    test_main()
