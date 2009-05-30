# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#
import os
import new
import sys

try:
    import xml.dom.minidom
    from xml.dom.ext.Dom2Sax import Dom2SaxParser
    import xml.xpath
except ImportError:
    # these are needed only for the documentation build
    pass

def normalize_cpp_id( id_ ):
    if id_.startswith('::'):
        return id_[2:]
    return id_

def nodeattr( node, key ):
    return node.attributes[(None,key)].nodeValue


###########################################################################
# saxhandler wraps
class saxhandler_with_filtering:
    """filters out specified elements"""
    def __init__( self, h, elements ):
        self.h = h
        self.elements = elements
        self.blocked = []

    def startElement( self, name, attrs ):
        if not self.blocked:
            if name in self.elements:
                self.blocked.append( name )
            else:
                self.h.startElement( name, attrs )

    def endElement( self, name ):
        if name in self.elements:
            assert self.blocked[-1] == name
            self.blocked.pop()
        elif not self.blocked:
            self.h.endElement( name )

    def characters( self, ch ):
        if not self.blocked:
            self.h.characters( ch )


class saxhandler_first_node:
    """lets through only the first node"""
    def __init__( self, h ):
        self.h = h
        self.s = []
        self.done = False

    def startElement( self, name, attrs ):
        if not self.done:
            self.s.append( name )
            self.h.startElement( name, attrs )

    def endElement( self, name ):
        if not self.done:
            self.h.endElement( name )
            self.s.pop()
            if not self.s:
                self.done = True

    def characters( self, ch ):
        if not self.done:
            self.h.characters( ch )

###########################################################################
# object hirerarchy
class doxybase(object):
    def __init__( self, node, doxy ):
        self.node_ = node
        self.doxy = doxy

    def is_internal(self):
        for xpath in ['detaileddescription/internal',
                      'briefdescription/internal']:
            if xml.xpath.Evaluate(xpath, self.node_):
                return True
        return False

    def is_publicly_documented(self):
        for xpath in ['detaileddescription//jag_undocumented',
                        'briefdescription//jag_undocumented']:
            if xml.xpath.Evaluate(xpath, self.node_):
                return False
        return True

    def _process_node( self, xpath, handler, allow_empty=False, first_child=True, take_parent=False ):
        nodes = xml.xpath.Evaluate(xpath, self.node_)
        if not nodes and allow_empty:
            return
        assert len(nodes)==1
        p = Dom2SaxParser()
        p.setContentHandler(handler)
        node = first_child and nodes[0].firstChild or nodes[0]
        if take_parent:
            node = node.parentNode
        p.parse(node)

    def detailed_desc(self, handler):
        hwrap = saxhandler_with_filtering(handler, set( ['simplesect', 'parameterlist'] ) )
        self._process_node( 'detaileddescription', hwrap )

    def brief_desc(self, handler):
        self._process_node( 'briefdescription', handler )

    def parameters_desc(self, handler):
        hwrap = saxhandler_first_node(handler)
        # @kind="exception" -> exceptions
        self._process_node( 'detaileddescription/para/parameterlist[@kind="param"]', hwrap, True, False )

    def simplesect_desc(self, handler, kind ):
        hwrap = saxhandler_first_node(handler)
        self._process_node( 'detaileddescription/para/simplesect[@kind="%s"]'%kind, hwrap, True, False )

    # To add a custom alias go to file doxyfile-template and adjust the
    # ALIAS option. The alias should look like this:
    #
    # ALIAS = jag_alias="\par My Alias.\n\xmlonly<jag_alias/>\endxmlonly"
    #
    # Details:
    #  If we use "@jag_alias my alias text" somewhere in the documentation
    #  then Doxygen inserts <jag_alias/> as a sibling of "my alias
    #  text". So we need to find <jag_alias/> element first and then
    #  process its parent.
    #
    def custom_alias_desc(self, handler, alias):
        hwrap = saxhandler_with_filtering(handler, set(['title', alias]))
        self._process_node( 'detaileddescription/para/simplesect[@kind="par"]/para/%s' %
                            alias, hwrap, True, False, take_parent=True )

    def id(self):
        return nodeattr( self.node_, 'id' )



class class_d(doxybase):
    def __init__( self, node, doxy ):
        doxybase.__init__( self, node, doxy )

    def method( self, name ):
        xpath = 'sectiondef/memberdef[@kind="function"]/name/text()'
        nodes = [ node for node in xml.xpath.Evaluate(xpath, self.node_) if node.nodeValue == name]
        assert len(nodes) == 1
        return memfun_d( nodes[0].parentNode.parentNode, self.doxy )



class memfun_d(doxybase):
    def __init__( self, node, doxy ):
        doxybase.__init__( self, node, doxy )
        assert self.node_.parentNode.nodeName == 'sectiondef'

    def section( self ):
        """ ret values: 'user-defined', 'public-func' """
        return nodeattr( self.node_.parentNode, 'kind' )

    def section_header( self ):
        assert self.section() == 'user-defined'
        return xml.xpath.Evaluate('header/text()', self.node_.parentNode)[0].nodeValue

    def section_desc( self, handler ):
        """returns node"""
        assert self.section() == 'user-defined'
        nodes = xml.xpath.Evaluate('description', self.node_.parentNode)
        if nodes:
            assert len(nodes) == 1
            node = nodes[0]
            p = Dom2SaxParser()
            p.setContentHandler(handler)
            p.parse(node.firstChild)




class enumeration_d(doxybase):
    def __init__( self, node, doxy ):
        doxybase.__init__( self, node, doxy )

    def value( self, value_str ):
        xpath = 'enumvalue/name/text()'
        nodes = [ node for node in xml.xpath.Evaluate(xpath, self.node_) if node.nodeValue == value_str]
        assert len(nodes) == 1
        return enum_value_d( nodes[0].parentNode.parentNode, self.doxy )



class enum_value_d(doxybase):
    def __init__( self, node, doxy ):
        doxybase.__init__( self, node, doxy )


class function_d(doxybase):
    def __init__( self, node, doxy ):
        doxybase.__init__( self, node, doxy )




###########################################################################
class Doxy:
    def __init__( self, dir_ ):
        self.dir_ = os.path.abspath( dir_ )
        self.cache = {}
        self.index = self._get_doc( 'index.xml' )

    def _get_doc( self, name ):
        if name not in self.cache:
            doc = xml.dom.minidom.parse(os.path.join(self.dir_, name)).documentElement
            self.cache[name] = doc
        return self.cache[name]

    def _generic_find( self, xpath, name ):
        name = normalize_cpp_id( name )
        nodes = [ node for node in xml.xpath.Evaluate(xpath, self.index) if node.nodeValue == name]
        if len(nodes) != 1:
            raise RuntimeError( "Doxy: number of nodes for '%s' is %d" % (name,len(nodes)) )
        return nodes[0].parentNode.parentNode

    def find_class( self, name ):
        xpath = 'compound[@kind="class"]/name/text()'
        cls_node = self._generic_find( xpath, name )
        xmlfile = nodeattr( cls_node, 'refid' ) + '.xml'
        doc = self._get_doc( xmlfile )
        return class_d( doc.childNodes[1], self )


    def _find_in_ns( self, type_, typedef, name ):
        xpath = 'compound[@kind="namespace"]/member[@kind="%s"]/name/text()' % type_
        ns, entity = normalize_cpp_id( name ).rsplit( '::', 1 )
        entity_node = self._generic_find( xpath, entity )
        ns_node = entity_node.parentNode
        assert xml.xpath.Evaluate("name/text()", ns_node)[0].nodeValue == ns
        ns_xmlfile = nodeattr( ns_node, 'refid' ) + '.xml'
        ns_doc = self._get_doc( ns_xmlfile )
        entity_id = nodeattr( entity_node, 'refid' )
        xpath = 'compounddef/sectiondef[@kind="%s"]/memberdef[@kind="%s"][@id="%s"]' % ( typedef, type_, entity_id )
        return xml.xpath.Evaluate(xpath, ns_doc)[0]


    def find_enumeration( self, name ):
        return enumeration_d( self._find_in_ns( 'enum', 'enum', name), self )

    def find_function( self, name ):
        return function_d( self._find_in_ns( 'function', 'func', name), self )


def get_comments_w32( doxycfg, xmldir ):
    for drive in ['c', 'd']:
        doxyexe = drive + ':/Progra~1/doxygen/bin/doxygen.exe'
        if os.path.isfile(doxyexe):
            break
    in_, out = os.popen4(doxyexe + ' -')
    in_.write( doxycfg )
    in_.close()
    doxyout = out.read()
    ret = out.close()
    if ret is None:
        return Doxy( xmldir ), doxyout
    else:
        raise RuntimeError( doxyout + 'Doxygen failed with code %d' % ret )
    pass

def get_comments_linux( doxycfg, xmldir ):
    in_, out = os.popen4( 'doxygen -' )
    in_.write( doxycfg )
    in_.close()
    doxyout = out.read()
    ret = out.close()
    if ret is None:
        return Doxy( xmldir ), doxyout
    else:
        raise RuntimeError( doxyout + 'Doxygen failed with code %d' % ret )
    pass




def get_comments( doxycfg, xmldir ):
    if sys.platform.startswith( 'linux' ):
        return get_comments_linux( doxycfg, xmldir )
    else:
        return get_comments_w32( doxycfg, xmldir )


###########################################################################
# test
def _dump_method( m ):
    print 'internal:', m.is_internal()
    print 'section:', m.section()
    if m.section()=='user-defined':
        print 'section header:', m.section_header()
        print 'section description:', m.section_desc()


if __name__ == "__main__":
    d = Doxy( 'c:/Code/cpp/sandbox/jagbase/code/tools/source_parsing/xml-jagbase' )
#     d = Doxy('c:/Code/python/sig/xml/')
#     c = d.find_class( "::Strs::DocPlatform::IStoreManager" )

#     for m_str in [ 'init_internal', 'typemapped' ]:
#         m = c.method( m_str )
#         print '>>', m_str
#         _dump_method( m )

#     e = d.find_enumeration( "::Strs::DocPlatform::RequestPriority" )
#     for val in ['UNDEFINED_internal', 'ASAP', 'NO_RUSH']:
#         ev = e.value( val )
#         print val, '-', ev.is_internal() and 'Internal' or 'Public'

#     f = d.find_function( "::Strs::DocPlatform::factory" )


