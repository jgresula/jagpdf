# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

from pygccxml import declarations

###########################################################################
# gccxml traits for intrusive pointer
g_value_type_cache = {}
g_top_parent = None

def lookup_value_type( cls ):
    global g_value_type_cache
    global g_top_parent

    if not g_top_parent:
        g_top_parent = cls.top_parent
    # check that the cache is used by the same parser instance
    assert id(g_top_parent) == id(cls.top_parent)
    # get the class name
    value_type_str = declarations.templates.args( cls.name )[0]
    if not value_type_str.startswith( '::' ):
        value_type_str = '::' + value_type_str
    # lookup in cache
    if value_type_str in g_value_type_cache:
        return g_value_type_cache[value_type_str]
    # search declarations and update cache
    r = cls.top_parent.decls( name=value_type_str
                              , function=lambda decl: not isinstance( decl, declarations.class_declaration.class_declaration_t )
                              ,  allow_empty=True )
    if len(r) != 1:
        raise RuntimeError( "Unable to find out intrusive_ptr value type. intrusive_ptr class is: %s" % cls.decl_string )
    g_value_type_cache[value_type_str] = r[0]
    return r[0]




class intrusive_ptr_traits:
    @staticmethod
    def _strip_type( type_ ):
        type_ = declarations.type_traits.remove_alias( type_ )
        type_ = declarations.type_traits.remove_reference( type_ )
        type_ = declarations.type_traits.remove_cv( type_ )
        return declarations.type_traits.remove_declarated( type_ )


    @staticmethod
    def match( type ):
        """returns True, if type represents instantiation of C{boost::intrusive_ptr}, False otherwise"""
        type = intrusive_ptr_traits._strip_type( type )
        if not isinstance( type, ( declarations.class_declaration.class_declaration_t, declarations.class_declaration.class_t ) ):
            return False
        if not declarations.type_traits.impl_details.is_defined_in_xxx( 'boost', type ):
            return False
        return type.decl_string.startswith( '::boost::intrusive_ptr<' )

    @staticmethod
    def value_type( type ):
        """returns reference to boost::shared_ptr value type"""
        if not intrusive_ptr_traits.match( type ):
            raise TypeError( 'Type "%s" is not instantiation of boost::intrusive_ptr' % type.decl_string )
        cls = intrusive_ptr_traits._strip_type( type )
        if isinstance( cls, declarations.class_declaration.class_t ):
            return declarations.type_traits.remove_declarated( cls.typedef( "element_type", recursive=False ).type )
        elif not isinstance( cls, ( declarations.class_declaration.class_declaration_t, declarations.class_declaration.class_t ) ):
            raise RuntimeError( "Unable to find out shared_ptr value type. shared_ptr class is: %s" % cls.decl_string )
        else:
            return lookup_value_type( cls )
#             value_type_str = declarations.templates.args( cls.name )[0]
#             if not value_type_str.startswith( '::' ):
#                 value_type_str = '::' + value_type_str
#             r = cls.top_parent.decls( name=value_type_str
#                                       , function=lambda decl: not isinstance( decl, declarations.class_declaration.class_declaration_t )
#                                       ,  allow_empty=True )
#             if len(r) != 1:
#                 raise RuntimeError( "Unable to find out intrusive_ptr value type. intrusive_ptr class is: %s" % cls.decl_string )
#             return r[0]
