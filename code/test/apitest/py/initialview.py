#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jag.testlib as testlib
import jagpdf

class CreatorBase:
    def __init__(self, option, value):
        self.profile = testlib.test_config()
        self.profile.set(option, value)
        self.profile.set('info.title', 'The title of this document')
        self.option = option
        self.value = value

    def __call__(self):
        return self.doc


class DocFromFile(CreatorBase):
    def __init__(self, argv, option, value, fname_stem=None):
        if fname_stem == None:
            fname_stem = '_' + value.lower()
        CreatorBase.__init__(self, option, value)
        doc_name = option.replace('.', '_') + fname_stem + '.pdf'
        self.doc = testlib.create_test_doc(argv, doc_name, self.profile)


class DocNoopStream(CreatorBase):
    def __init__(self, option, value):
        CreatorBase.__init__(self, option, value)
        self.doc = jagpdf.create_stream(testlib.NoopStreamOut(), self.profile)

#
#
#
def do_document(doc_creator):
    doc = doc_creator()
    for i in range(5):
        doc.page_start(500, 800)
        doc.outline().item("Bookmark #%d" % (i+1))
        doc.page().canvas().text(20, 720, "Page %s, initial view - %s = %s" %
                                 (i+1, doc_creator.option, doc_creator.value))
        doc.page_end()
    doc.finalize()

#
#
#
def test_main(argv=None):
    #
    # PageLayout
    #
    # TwoPageLeft and TwoPageRight not supported now
    for val in ['SinglePage',   # single page
                'OneColumn',    # continuous
                'TwoColumnLeft', # continuous facing
                'TwoColumnRight']:  # continuous facing (requires numbering)
        creator = DocFromFile(argv, 'doc.page_layout', val)
        do_document(creator)
    testlib.must_throw(do_document, DocNoopStream('doc.page_layout', 'InvalidValue'))
    #
    # PageMode
    #
    for val in ['UseNone', 'UseOutlines', 'UseThumbs', 'FullScreen']:
        creator = DocFromFile(argv, 'doc.page_mode', val)
        do_document(creator)
    testlib.must_throw(do_document, DocNoopStream('doc.page_mode', 'InvalidValue'))
    #
    # InitialDestination
    #
    creator = DocFromFile(argv, "doc.initial_destination",
                          "mode=XYZ; zoom=1.5",
                          "")
    do_document(creator)
    creator = DocFromFile(argv, "doc.initial_destination", "page=4; mode=XYZ", "_1")
    do_document(creator)
    testlib.must_throw(
        do_document, DocNoopStream("doc.initial_destination", "page=5; mode=XYZ"))
    #
    # ViewePreferences
    #
    for val in ['HideToolbar',
                'HideMenubar',
                'HideWindowUI',
                'FitWindow',
                'CenterWindow',
                'DisplayDocTitle']:
        creator = DocFromFile(argv, "doc.viewer_preferences", val)
        do_document(creator)
        creator = DocFromFile(argv,
                              "doc.viewer_preferences",
                              "HideToolbar; CenterWindow; DisplayDocTitle",
                              "_combined")
        do_document(creator)




if __name__ == "__main__":
    test_main()
