#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import jagpdf
import jag.testlib as testlib
import sys
import os
import threading

s_profile ="""
info.title = title
info.author = unknown
info.subject = unknown
info.keywords = unknown
info.creator = unknown
info.creation_date = unknown
"""

s_image_dir = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}')
s_jpeg_file = os.path.join(s_image_dir, "images-jpeg",
                            "PalmTree-CMYK-icc-FOGRA27.jpg")

def test_main(argv=None):
    do_it()
    num_threads = 10
    docs_per_thread = 3;
    while 1:
        threads = [WorkerThread(docs_per_thread) for i in xrange(num_threads)]
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()

class WorkerThread(threading.Thread):
    def __init__(self, num_docs):
        threading.Thread.__init__(self)
        self.num_docs = num_docs

    def run(self):
        while self.num_docs:
            do_it()
            self.num_docs -=  1

def do_it():
    prof = jagpdf.create_profile_from_string(s_profile)
    stream = testlib.NoopStreamOut()
    doc = jagpdf.create_stream(stream)
    doc.page_start(5.9*72, 3.5*72)
    canvas = doc.page().canvas()
    # meat
    img = doc.image_load_file(s_jpeg_file)
    canvas.image(img, 50, 50)
    canvas.text(10, 10, 200 * 'a')
    canvas.move_to(10, 10)
    canvas.line_to(20, 20)
    canvas.path_paint("fs")
    font_ttf = testlib.EasyFontTTF(doc)(10)
    font_core = testlib.EasyFont(doc)(10)
    canvas.text_font(font_ttf)
    canvas.text(10, 10, 50 * 'x')
    font_ttf.advance('abcdefghijklmnopqrstuvwxyz')
    canvas.text_font(font_core)
    canvas.text(10, 10, 50 * 'y')
    font_core.advance('abcdefghijklmnopqrstuvwxyz')
    # finalize
    doc.page_end()
    doc.finalize()
    # touch the result
    s = 0
    for b in stream.content():
        s = s + ord(b)

if __name__ == "__main__":
    test_main()

