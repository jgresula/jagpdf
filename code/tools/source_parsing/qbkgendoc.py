# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

from pygccxml import declarations
import re
import os
import string
import StringIO
import introspect
import sig
import shutil
import sys
import textwrap
import md5


###########################################################################

class Bunch:
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

def main( cfg, intro ):
    #QuickBook(4).generate(cfg)
    make_quickbook_doc(cfg, True)



template_file_qbk="""[article ref_generated
    [quickbook 1.4]
]

[/ ===================================== ]
[section:classes Classes]
[/ ===================================== ]
${classes_doc}
[endsect]

[/ ===================================== ]
[section:functions Functions]
[/ ===================================== ]
${functions_doc}
[endsect]

[/ ===================================== ]
[section:enums Enums]
[/ ===================================== ]

[include enumsintro.qbk]

${enums_doc}
[endsect]

"""
def  make_quickbook_doc( opts, run_doxy=True ):
    if not os.path.isdir( opts['qbkoutdir'] ):
        os.makedirs( opts['qbkoutdir'] )
    rtti = introspect.introspect( opts, run_doxy )
    ref_str = make_reference_map(rtti, opts)
    enums_doc = enums_to_qbk( rtti, opts )
    functions_doc = functions_to_qbk( rtti, opts )
    classes_doc = classes_to_qbk(rtti, opts )
    file(os.path.join(opts['qbkoutdir'], "generated_doc.qbk"), "wb" ).write(string.Template(template_file_qbk).substitute(locals()))
    file(os.path.join(opts['qbkoutdir'], "generated_ref_links.qbk"), "wb").write(ref_str)

def transform_id( o ):
    m=md5.new()
    m.update(o.id())
    return 'id_'+m.hexdigest()

def make_reference_map(rtti, opts):
    """adds 'ref_map' key which maps doxygen id to a quickbook
template. In addition it creates a qbk stream with templates for
generated references"""
    tmpl_class="""
[/ -------------------------------------------------------------------------- ]
[/ Class ${cls_name} ]
[template ${cls_ref}[][link ${cls_id} ${cls_name}]]
[template ${cls_ref}_t[txt][link ${cls_id} [txt]]]
[/ -------------------------------------------------------------------------- ]
"""
    tmpl_method="""
[template ${meth_ref}[][link ${meth_id} ${meth_name}()]]
[template ${meth_ref}_t[txt][link ${meth_id} [txt]]]
"""
    tmpl_freefun="""
[template ${fun_ref}[][link ${fun_id} ${fun_name}()]]
[template ${fun_ref}_t[txt][link ${fun_id} [txt]]]
"""
    tmpl_enum="""
[template ${enum_ref}[][link ${enum_id} ${enum_name}]]
[template ${enum_ref}_t[txt][link ${enum_id} [txt]]]
"""
    stream = StringIO.StringIO()
    m = {}
    for cls in [c for c in rtti.exported_classes() if c.is_publicly_documented()]:
        cls_id = 'ref_generated.reference.classes.' + transform_id(cls)
        cls_name = opts['doc_cls_name'](cls.gcc)
        cls_ref = 'code_'+cls_name.lower()
        stream.write(string.Template(tmpl_class).substitute(locals()))
        m[cls.id()]=Bunch(ref=cls_ref,id=cls_id)
        for meth in [me for me in cls.methods() if me.is_publicly_documented()]:
            meth_id=transform_id(meth)
            meth_name=meth.remapped_name()
            meth_ref='code_'+cls_name.lower()+'_'+meth.name().lower()
            stream.write(string.Template(tmpl_method).substitute(locals()))
            m[meth.id()]=Bunch(ref=meth_ref,id=meth_id)
    for enum in [e for e in rtti.exported_enums() if e.is_publicly_documented()]:
        enum_id='ref_generated.reference.enums.'+transform_id(enum)
        enum_name=enum.name()
        enum_ref='code_'+enum_name.lower()
        stream.write(string.Template(tmpl_enum).substitute(locals()))
        m[enum.id()]=Bunch(ref=enum_ref,id=enum_id)
    for fun in [f for f in rtti.exported_freefunctions() if f.is_publicly_documented()]:
        fun_id='ref_generated.reference.functions.'+transform_id(fun)
        fun_name=fun.name()
        fun_ref='code_'+fun_name.lower()
        stream.write(string.Template(tmpl_freefun).substitute(locals()))
        m[fun.id()]=Bunch(ref=fun_ref,id=fun_id)
    opts['ref_map']=m
    return stream.getvalue()


def qbk_link_template(opts, id_, txt=None):
    ref=opts['ref_map'][id_].ref
    if txt:
        return '[%s_t %s]' % (ref, txt)
    else:
        return '[' + ref +']'

def qbk_link_id(opts, id_):
    return opts['ref_map'][id_].id


class DummyHandler:
    def __init__(self):
        self.w = StringIO.StringIO()
    def content(self):
        return self.w.getvalue()
    def characters(self, chars):
        self.w.write(chars)
    def startElement(self, name, attrs):
        self.w.write('<'+name+'>')
    def endElement(self, name):
        self.w.write('</'+name+'>')



class StreamsStack:
    PROLOGUE, MAIN = range(2)
    def __init__(self):
        self.streams = [StringIO.StringIO(), StringIO.StringIO()]
        self.stream = [self.streams[StreamsStack.MAIN]]

    def content(self):
        return '\n'.join([s.getvalue() for s in self.streams])

    def write(self,str):
        self.stream[-1].write(str)

    def push_stream(self, stream):
        self.stream.append(self.streams[stream])

    def pop_stream(self):
        self.stream.pop()
        assert len(self.stream) >= 1


class HandlerBaseImpl:
    def __init__(self, stream, opts):
        self.stream = stream
        self.opts = opts

    def write(self,str):
        self.stream.write(str)

    def push_stream(self, w):
        self.stream.push_stream(w)

    def pop_stream(self):
        self.stream.pop_stream()

    def on_activate(self,ctx):
        pass

    def on_deactivate(self,ctx):
        pass


class BaseHandler(HandlerBaseImpl):
    def __init__(self, stream, opts):
        HandlerBaseImpl.__init__(self, stream, opts)
        self.chars=''

    def flush_chars(self):
        chars = self.chars
        if chars:
#            self.stream.write('\n'.join(textwrap.wrap(chars,76)))
            self.write(chars)
            self.chars=''
        return chars

    def on_chars(self,chars):
        self.chars += chars

    def on_para_start(self,attr):
        pass

    def on_para_end(self):
        self.write( '\n\n' )
        self.write( '\n\n' )

class Counter:
    def __init__(self):
        self.cnt = 0

    def inc(self):
        self.cnt += 1
        return self.cnt

g_list_counter = Counter()

# List handling is tricky as at this moment there is no way how to
# place a list inside a table or a variable list (see:
# http://thread.gmane.org/gmane.comp.lib.boost.documentation/3495)
#
# The only way is generate a template (block phrase) and instantiate
# it. So when a list is start we switch to a prologue stream and
# define a template with a list there. Once the list is done we switch
# back to the main stream and instantiate the template there.
#
class ListHandler(HandlerBaseImpl):
    def __init__(self,stream, opts):
        HandlerBaseImpl.__init__(self,stream, opts)
        self.list_id = None

    def on_activate(self,ctx):
        if ctx['list-depth'] == -1:
            self.push_stream(StreamsStack.PROLOGUE)
            self.list_id = "templ_list_%03d" % g_list_counter.inc()
            self.write('\n[template %s[]\n' % self.list_id)
        ctx['list-depth']+=1
        self.spaces = ctx['list-depth']*' '

    def on_deactivate(self,ctx):
        ctx['list-depth'] -= 1
        if ctx['list-depth'] == -1:
            self.write('\n]\n')
            self.pop_stream()
            assert self.list_id
            self.write('[%s]\n' % self.list_id)

    def on_itemizedlist_start(self,attr):
        self.write('\n')

    def on_itemizedlist_end(self):
        pass

    def on_listitem_start(self,attr):
        self.write( self.spaces+'* ' )

    def on_listitem_end(self):
        self.write('\n')



class RefHandler(HandlerBaseImpl):
    def __init__(self,stream, opts):
        HandlerBaseImpl.__init__(self, stream, opts)
        self.chars=''

    def on_chars(self,chars):
        self.chars += chars

    def on_ref_start(self,attr):
        our_link = qbk_link_id(self.opts, attr['refid'])
        self.write( ' [link %s ' % our_link )

    def on_ref_end(self):
        # strip qualification from links
        self.write(self.chars.split('::')[-1])
        self.write( ']' )



class URLHandler(HandlerBaseImpl):
    def __init__(self,stream, opts):
        HandlerBaseImpl.__init__(self, stream, opts)

    def on_ulink_start(self, attr):
        self.write(' [@')

    def on_ulink_end(self):
        self.write(']')



class ParamHandler(HandlerBaseImpl):
    def __init__(self, stream, opts):
        HandlerBaseImpl.__init__(self, stream, opts)

    def on_parameterlist_start(self,attr):
        #self.write( '<parameterlist>' )
        self.write('[variablelist\n')

    def on_parameterlist_end(self):
        #self.write( '</parameterlist>' )
        self.write(']\n') # end variable list and phrase

    def on_parameteritem_start(self,attr):
        #self.write( '<parameteritem>' )
        self.write('[') # item start

    def on_parameteritem_end(self):
        #self.write( '</parameteritem>' )
        self.write(']\n') # item end

    def on_parameternamelist_start(self,attr):
        #self.write( '<parameternamelist>' )
        self.write('[') # name start

    def on_parameternamelist_end(self):
        #self.write( '</parameternamelist>' )
        self.write(']') # name end

    def on_parametername_start(self,attr):
        #self.write( '<parametername>' )
        pass

    def on_parametername_end(self):
        #self.write( '</parametername>' )
        pass

    def on_parameterdescription_start(self,attr):
        #self.write( '<parameterdescription>' )
        self.write('[') # value start

    def on_parameterdescription_end(self):
        #self.write( '</parameterdescription>' )
        self.write(']') # value end





class HandlerStack:
    handler_map = { 'itemizedlist' : ListHandler,
                    'ref' : RefHandler,
                    'parameterlist' : ParamHandler,
                    'ulink' : URLHandler }
    def __init__(self, opts):
        self.stream = StreamsStack()
        self.ctx = { 'list-depth' : -1 }
        self.stack = [BaseHandler(self.stream, opts)]
        self.opts = opts

    def flush_chars(self):
        self.stack[0].flush_chars()

    def push(self, name):
        if name in HandlerStack.handler_map:
            handler = HandlerStack.handler_map[name](self.stream, self.opts)
            handler.on_activate(self.ctx)
            self.stack.append(handler)

    def pop(self, name):
        if name in HandlerStack.handler_map:
            self.stack[-1].on_deactivate(self.ctx)
            self.stack.pop()


class DocHandler:
    """Forms a sequence of handlers and dispatches events to them."""
    def __init__(self, opts):
        self.handlers = HandlerStack(opts)
        # internal needed for some reason (doxygen 1.5.3) note, that
        # in this case our @internal entities are not recoginized
        self.ignored_elements = set(['internal'])

    def content(self):
        return self.handlers.stream.content()

    def add_ignored(self,name):
        self.ignored_elements.add(name)

    def remove_ignored(self, name):
        self.ignored_elements.remove(name)

    def find_handler(self, element):
        for h in reversed(self.handlers.stack):
            name='on_'+element
            if hasattr(h, name):
                return getattr(h,name)
        raise RuntimeError( 'No handler for %s.' % element )

    def characters(self, chars):
        self.find_handler('chars')(chars)

    def startElement(self, name, attrs):
        self.handlers.flush_chars()
        if name in self.ignored_elements:
            return
        self.handlers.push( name )
        self.find_handler(name+'_start')(attrs)

    def endElement(self, name):
        self.handlers.flush_chars()
        if name in self.ignored_elements:
            return
        self.find_handler(name+'_end')()
        self.handlers.pop(name)

def enums_to_qbk( rtti, opts ):
    template_enum_qbk="""
[/ ------------------------------------------------------------------- ]
[section:${enum_id} [phrase jag_funtitle Enum ${enum_name}]]
[/ ------------------------------------------------------------------- ]

${values}

[endsect]
"""
    stream = StringIO.StringIO()
    exp_enums = [ (e.name(),e) for e in rtti.exported_enums() if e.is_publicly_documented() ]
    exp_enums.sort()
    for enum_name, enum in exp_enums:
        enum_id = transform_id(enum)
        values = "\n".join(['* `%s`' % n for n, v in enum.documented_values()])
        stream.write(string.Template(template_enum_qbk).substitute(locals()))
    return stream.getvalue()



def classes_to_qbk( rtti, opts ):
    template_class_qbk="""
[/ ------------------------------------------------------------------- ]
[section:${class_id} [phrase jag_classtitle Class ${class_name}]]
[/ ------------------------------------------------------------------- ]
[heading Description]
${class_description}

[heading Synopsis]
[phrase jag_synopsis
```
class ${class_name}
{
public:
${class_body}};
```
]

[heading Methods]
${class_methods}
[endsect]
"""
    def try_rename(cls, m):
        """this function ensures nice sorting of methods in class
        synopsis"""
        try:
            result = opts['rename'][cls.gcc.decl_string[2:] + '::' + m.name()]
        except KeyError:
            result = m.name()
        args = [arg.type.decl_string + arg.name for arg in m.gcc.arguments]
        return result + ', '.join(args)
    stream = StringIO.StringIO()
    exp_classes = [(opts['doc_cls_name'](cls.gcc), cls) for cls in rtti.exported_classes() if cls.is_publicly_documented()]
    exp_classes.sort()
    for class_name, cls in exp_classes:
        methods = [(try_rename(cls, m), m) for m in cls.methods() if m.is_publicly_documented()]
        methods.sort()
        methods = [m for name,m in methods]
        class_body=cls_synopsis_to_qbk(methods, opts)
        class_methods=cls_methods_to_qbk(cls, class_name, methods, opts)
        handler = DocHandler(opts)
        handler.add_ignored('para')
        cls.brief_desc(handler)
        handler.remove_ignored('para')
        cls.detailed_desc(handler)
        class_description = handler.content()
        class_id = transform_id(cls)
        stream.write(string.Template(template_class_qbk).substitute(locals()))
    return stream.getvalue()


def cls_methods_to_qbk(cls, class_name, methods, opts):
    template_callable_qbk="""
[#${callable_id}]
[phrase jag_funtitle [link ${class_id} ${class_name}]::[link ${callable_id} ${fn_name}]]
%s
"""
    return callables_to_qbk( methods, opts,
                             template_callable_qbk,
                             class_id=qbk_link_id(opts, cls.id()),
                             class_name=class_name)


def functions_to_qbk( rtti, opts ):
    function_callable_qbk="""
[section:${callable_id} [phrase jag_funtitle Function ${fn_name}()]]
%s
[endsect]
"""
    funs = [f for f in rtti.exported_freefunctions() if f.is_publicly_documented()]
    funs.sort(lambda l,r: cmp(l.name(),r.name()))
    return callables_to_qbk( funs, opts, function_callable_qbk )


def callables_to_qbk( callables, opts, template, **kwds ):
    template_callable="""[phrase jag_fundoc
```
${sigs_str}
```
]
${effect}

${precondition}

${params}

${pdfversion}

${myversion}

${see}
"""
    stream = StringIO.StringIO()
    def get_prefix(name, content):
        lbreak = ''
        if content.startswith('*'): # list must start on a new line
            lbreak = '\n\n'
        return '[phrase jag_doc_item %s: ]%s%s' % (name, lbreak, content)

    def process_simplesect(c, tag, prefix):
        handler = DocHandler(opts)
        handler.add_ignored('simplesect')
        c.simplesect_desc(handler, tag)
        content =  handler.content().strip()
        if content and prefix:
            content = get_prefix(prefix, content)
        return content


    for call_able in callables:
        callable_id = transform_id(call_able)
        try:
            fn_name = call_able.remapped_name()
        except AttributeError:
            fn_name = call_able.name()
        sigs = callable_signature_to_qbk(opts,
                                         call_able,
                                         ['cpp','c','py', 'java'],
                                         name_link=False,
                                         wrap_info=Bunch(offset=7 ,length=90))
        sigs_list = []
        for lang, tag in [('py',  '[py]   '),
                          ('cpp', '[c++]  '),
                          ('c',   '[c]    '),
                          ('java','[java] ')]:
            if lang in sigs:
                sigs_list.append(tag + sigs[lang])
        sigs_str = ''.join(sigs_list)
        # brief + detailed sections
        handler = DocHandler(opts)
        handler.add_ignored('para')
        call_able.brief_desc(handler)
        handler.remove_ignored('para')
        call_able.detailed_desc(handler)
        effect = get_prefix('Effect', handler.content().strip())
        # parameters
        handler = DocHandler(opts)
        call_able.parameters_desc(handler)
        params = handler.content().strip()
        if params:
            params = get_prefix('Parameters', params)
        # simple sections
        see = process_simplesect(call_able, 'see', 'See')
        precondition = process_simplesect(call_able, 'pre', 'Precondition')
        myversion = process_simplesect(call_able, 'version', '[lib] version')
        # pdf version;
        handler = DocHandler(opts)
        pdfversion = call_able.custom_alias_desc(handler, 'jag_pdfversion')
        pdfversion = handler.content().strip()
        if pdfversion:
            pdfversion = get_prefix('PDF version', pdfversion)
        # substitute and write
        d = dict(kwds)
        d.update(locals())
        stream.write(string.Template(template%template_callable).substitute(d))
    return stream.getvalue()



def cls_synopsis_to_qbk(methods, opts):
    stream = StringIO.StringIO()
    for meth in methods:
        sigs = callable_signature_to_qbk(opts, meth, ['cpp'],
                                         wrap_info=Bunch(offset=2,length=100))
        stream.write('  '+ sigs['cpp'])
    return stream.getvalue()


def callable_signature_to_qbk( opts, callable_, langs, name_link=True, wrap_info=None ):
    class sig_handler:
        def __init__(self,callable_,name_link):
            self.callable=callable_
            self.name_link=name_link
        def __call__(self,stream):
            self.stream=stream
            return self
        def write( self, str_ ):
            self.stream.write( str_ )
        def on_name( self, name ):
            self.stream.write( "``'''<phrase role=\"jag_callable_name\">'''``" )
            if self.name_link:
                self.stream.write( '``'+qbk_link_template(opts,self.callable.id(),name)+'``')
            else:
                self.stream.write(name)
            self.stream.write( "``'''</phrase>'''``" )
        def on_link_start( self, lnk ):
            self.stream.write( '``[link %s ' % qbk_link_id(opts,lnk) )
        def on_link_end( self ):
            self.stream.write( ']``' )
    sig_map = { 'cpp' : (sig.cpp_signature,'%s;\n'),
                'c'   : (sig.c_signature,  '%s;\n'),
                'py'  : (sig.py_signature, '%s\n'),
                'java' : (sig.java_signature, '%s;\n')}
    sigs = []
    for lang in langs:
        sig_t, sig_fmt = sig_map[lang]
        s = sig_t( opts, True )
        # From historical reasons, signature.get() retrieves the
        # signature handler parameter as a type. However, we pass it
        # here as an instance of sig_handler as we need to initialize
        # it with the corresponding callable object. So
        # signature.get() thinks it calls a constructor, but actually
        # __call__ is called.
        sig_full = s.get(callable_, sig_handler(callable_,name_link))
        if sig_full == 'n/a':
            sig_full += '\n'
        else:
            sig_full = sig_fmt % sig_full
            if lang!='py' and wrap_info:
                sig_raw = s.get(callable_)
                breaks = signature_breaks(sig_raw, wrap_info)
                sig_full = wrap_signature(sig_full, wrap_info, breaks, sig_raw)
        sigs.append((lang, sig_full))
        # merge consecutive escapes to quickbook
        sigs = [ (lang, s.replace('````', '')) for lang, s in sigs ]
    return dict(sigs)


def wrap_signature( sig, wrap_spec, breaks, sig_raw ):
    assert(' ' in sig)
    s = []
    parenti = sig_raw.find('(')+1
    indent=',\n'+(parenti+wrap_spec.offset)*' '
    spans=sig.split( ',' )
    action=lambda s: s
    for i, piece in enumerate(spans):
        piece=action(piece)
        s.append(piece)
        if i in breaks:
            s.append(indent)
            action=lambda s: s.lstrip()
        elif i+1<len(spans):
            s.append(',')
            action=lambda s: s
    return ''.join(s)

rex_args = re.compile('(\(.*)')
def signature_breaks(sig, wrap_spec):
    def span_len(i):
        if i+1<len(spans):
            return len(spans[i])+1
        else:
            return len(spans[i])
    width = wrap_spec.length-(wrap_spec.offset+sig.find('('))+1
    breaks = []
    curr_width=0
    args=rex_args.findall(sig)
    assert(len(args)==1)
    spans=args[0].split(',')
    for i, span in enumerate(spans):
        if curr_width+span_len(i) > width:
            assert(i>0) # too long function name, no emergency mode, just assert
            breaks.append(i-1)
            curr_width=span_len(i)
        else:
            curr_width+=span_len(i)
    return breaks




