#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

__doc__ = """Reads from stdin an replaces matches from htmlfilt.conf with an
empty string. Sends the result to stdout."""

import sys
import os
import re
import xml.xpath
import xml.dom.minidom
import StringIO
import logging

LOG_FILENAME = '/tmp/htmlfilt.log'
logging.basicConfig(filename=LOG_FILENAME,
                    level=logging.DEBUG, #logging.DEBUG, logging.INFO
                    format='%(message)s',
                    filemode='w')


#
rex_html=re.compile("<head>.*</body>", re.M | re.S)
def skip(data):
    data = data.replace('&nbsp;', ' ') # expat fix
    try:
        doc = xml.dom.minidom.parseString(data)
        xpaths = ['*//div[@class="programlisting"]',
#                  "*//a[contains(@href,'ref_generated.reference')]",
                  "*//a[contains(@href,'.id_')]",
                  "*//a[contains(@href,'#id_')]",
                  "*//span[@class='identifier']",
                  "*//span[@class='jag_callable_name']",
                  "*//span[@class='jag_fuqntitle']"
                  ]
        for xpath in xpaths:
            nodes = xml.xpath.Evaluate(xpath, doc)
            for node in nodes:
                node.parentNode.removeChild(node)
        xml_s = doc.toprettyxml(indent="    ", encoding='utf-8')
        return rex_html.search(xml_s).group()
    except xml.parsers.expat.ExpatError:
        #not xml
        return data

rex_sep=re.compile(u'[\w.@:/-]+', re.U)
def tokenize(data):
    data = unicode(data, 'utf-8')
    i = 0
    while 1:
        m = rex_sep.search(data, i)
        if m:
            s = data[i:m.start()].encode('utf-8')
            sys.stdout.write(s)
            logging.debug('SEP  ' + s)
            yield data[m.start():m.end()]
            i = m.end()
        else:
            sep = data[i:].encode('utf-8')
            logging.debug('SEP  ' + sep)
            sys.stdout.write(sep)
            break

def main(data):
    data = skip(data)

    result = []
    this_dir = os.path.dirname(sys.argv[0])
    conf = os.path.join(this_dir, 'htmlfilt.conf')

    rexes = []
    for line in open(conf):
        line = line.strip()
        if line and not line.startswith('#'):
            rexes.append(line)
    rex_str = unicode('^[("]*(' + '|'.join(rexes) + ')[".?),]*$', 'utf-8')
    rex = re.compile(rex_str, re.U)

    for word in tokenize(data):
        m = rex.match(word)
        word_utf8 = word.encode('utf-8')
        if not m:
            sys.stdout.write(word_utf8)
        logging.debug((m and 'MATCH ' or 'PASS ') + word_utf8)


if __name__ == "__main__":
    main(sys.stdin.read())

    
