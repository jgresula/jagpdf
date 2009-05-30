#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import gencommon
import jagbasecfg
import introspect
import sig
import heapq
from string import Template
from pygccxml import declarations

cpp_class_body="""
class ${cpp_name}
{
    ${c_handle} m_obj;

public:
${body}

public: // operators + destructor
#if defined(_MANAGED)
    static void unspecified_bool(${cpp_name}***)
    {
    }

    typedef void (*unspecified_bool_type)(${cpp_name}***);
    operator unspecified_bool_type() const { // never throws
        return m_obj==0? 0: unspecified_bool;
    }
#else
    typedef ${c_handle} ${cpp_name}::*unspecified_bool_type;
    operator unspecified_bool_type() const { // never throws
        return m_obj==0? 0: &${cpp_name}::m_obj;
    }
#endif

    // operator! is redundant, but some compilers need it
    bool operator! () const { // never throws
        return m_obj==0;
    }

    %s

    ${cpp_name}()
        : m_obj(0)
    {}

public: // not to be used outside this file
    explicit ${cpp_name}(${c_handle} obj)
        : m_obj(obj)
    {}

    ${c_handle} handle_() {
        return m_obj;
    }
};
"""

cpp_class_ctordtor_refcount="""
    ~${cpp_name}() {
        ${c_handle_dec}(m_obj);
    }

    ${cpp_name}(${cpp_name} const& other) {
        m_obj = other.m_obj;
        ${c_handle_inc}(m_obj);
    }

    ${cpp_name}& operator=(${cpp_name} const& other) {
        ${c_handle_dec}(m_obj);
        m_obj = other.m_obj;
        ${c_handle_inc}(m_obj);
        return *this;
    }
"""

cpp_class_ctordtor="""
    ~${cpp_name}() {
    }

    ${cpp_name}(${cpp_name} const& other) {
        m_obj = other.m_obj;
    }

    ${cpp_name}& operator=(${cpp_name} const& other) {
        m_obj = other.m_obj;
        return *this;
    }

"""

def get_class_body(refcounted):
    if refcounted:
        return cpp_class_body % cpp_class_ctordtor_refcount
    else:
        return cpp_class_body % cpp_class_ctordtor


g_callable_muster_ptr = """${signature}
{
#ifndef ${no_exceptions_tag}
    ${c_result_cls} obj_ = ${invocation};
    if (!obj_)
        throw ${exception_instance};
    return ${cpp_result_cls}(obj_);
#else
    return ${cpp_result_cls}(${invocation});
#endif
}
"""
g_callable_muster_err_code = """${signature}
{
#ifndef ${no_exceptions_tag}
    if (${invocation})
        throw ${exception_instance};
#else
    return ${invocation};
#endif
}
"""

g_callable_muster_other = """${signature}
{
#ifndef ${no_exceptions_tag}
    ${return_type} result_ = ${invocation};
    if (${global_error_check})
        throw ${exception_instance};
    return result_;
#else
    return ${invocation};
#endif
}
"""


def cpp_indent(fragment, spaces=4):
    result = []
    for line in fragment.split('\n'):
        if line and  '#' != line[0]:
            result.append(spaces*' ' + line)
        else:
            result.append(line)
    return "\n".join(result)


def c_enum_name(enum, cfg):
    visitor = sig.type_visitor('c', cfg)
    return sig.apply_type_metainfo_visitor(enum, visitor)


def prepare_args(callable_, cfg):
    args = []
    cargs = callable_.args()
    for i, arg in enumerate(callable_.args()):
        if arg['category'] == 'interface':
            args.append('%s.handle_()' % cargs[i]['argname'])
        elif arg['category'] == 'enum':
            args.append('static_cast<%s>(%s)' % (c_enum_name(cargs[i], cfg),
                                                   cargs[i]['argname']))
        else:
            args.append(cargs[i]['argname'])
    return ', '.join(args)


def form_invocation(callable_, cfg):
    arglist = prepare_args(callable_, cfg )
    if callable_.is_memfun():
        if arglist:
            inv_str = "%s(m_obj, %s)"
        else:
            inv_str = "%s(m_obj%s)"
        call_name = cfg['c_method_name'](callable_.callable_object())
    else:
        inv_str = "%s(%s)"
        call_name = cfg['c_freefun_name'](callable_.callable_object())
    return inv_str % (call_name, arglist)


def callable_body_returning_void(callable_, cfg):
    return dict(invocation=form_invocation(callable_,cfg))


def callable_body_returning_interface(callable_, cfg):
    cls = callable_.return_type()['base']
    return dict(invocation=form_invocation(callable_,cfg),
                 c_result_cls=gencommon.c_handle_from_cls(cls,cfg),
                 cpp_result_cls=gencommon.cls_from_cls(cls,cfg))

def callable_body_returning_other(callable_, cfg):
    ret = callable_.return_type()['base']
    return_type = gencommon.map_type(callable_.return_type(), 'cpp', cfg)
    invocation=form_invocation(callable_,cfg)
    if declarations.type_traits.is_enum(ret):
        invocation='static_cast<%s>(%s)' % (return_type, invocation)
    return locals()

def cpp_callable_body(callable, cfg):
    """writes body of a free fun"""
    body_disp = { 'interface' : g_callable_muster_ptr,
                  'void' : g_callable_muster_err_code,
                  'other' : g_callable_muster_other }
    args = []
    mi = callable.metainfo()
    ret_cat = mi.return_type_category()
    body_dict = globals()['callable_body_returning_'+ret_cat](mi, cfg)
    gencommon.update_dict(body_dict, cfg, 'no_exceptions_tag', 'exception_instance', 'global_error_check')
    return body_dict, body_disp[ret_cat]


def output_callable(callable, cfg):
    s = sig.cpp_signature(cfg)
    csig = s.get(callable)
    if isinstance(callable, introspect.freefun_i):
        csig = 'inline ' + csig
    subst_dict, templ = cpp_callable_body(callable, cfg)
    subst_dict['signature']=csig
    return Template(templ).substitute(subst_dict)


def output_class(cls, header, intro, cfg):
    substd = { 'c_handle' : gencommon.c_handle_from_cls(cls,cfg),
               'cpp_name' : gencommon.cls_from_cls(cls,cfg) }
    gencommon.update_dict(substd, cfg, 'c_handle_inc', 'c_handle_dec')
    decl_string = gencommon.decl_string_generic(cls)
    cls_body = get_class_body(decl_string in cfg['ref_counted'])
    cls_muster = Template(cls_body).safe_substitute(substd)
    body = []
    import sig
    s = sig.cpp_signature(cfg)
    methods = [(s.get(m), m) for m in cls.methods()]
    methods.sort()
    for sig, m in methods:
        body.append(cpp_indent(output_callable(m, cfg), 4))
    substd = { 'body' : "\n".join(body) }
    cls_def = Template(cls_muster).substitute(substd)
    header.write(cls_def)


def output_free_funs(header, intro, cfg):
    header.write('\n\n\n/* ==== free functions ==== */\n\n')
    import sig
    s = sig.cpp_signature(cfg)
    funcs = [(s.get(f), f) for f in intro.exported_freefunctions()]
    funcs.sort()
    for sig, fun in funcs:
        header.write(output_callable(fun, cfg) + '\n\n')


# def output_classes_fw(header, intro, cfg):
#     header.write('\n\n/* ==== fw decls ==== */\n')
#     for cls in intro.exported_classes():
#         header.write("class %s;" % gencommon.cls_from_cls(cls,cfg))
#     header.write('\n\n')


def output_classes(header, intro, cfg):
    header.write('\n\n/* ==== classes ==== */\n')
    classes = []
    pre = cfg['cpp_cls_order']
    for cls in intro.exported_classes():
        try:
            qname = introspect.qualified_name(cls.gcc)
            heapq.heappush(classes, (pre[qname], qname, cls))
        except KeyError:
            heapq.heappush(classes, (10000, qname, cls))
    # sort classes
    cls_l = []
    while classes:
        p, qname, cls = heapq.heappop(classes)
        cls_l.append((p, qname, cls))
    cls_l.sort()
    for p, qname, cls in cls_l:
        output_class(cls, header, intro, cfg)


def gen_cpp_api(intro, cfg):
    streams = gencommon.get_streams(cfg, 'cpp_header')
    gencommon.output_enums(streams['cpp_header'], intro, cfg, 'cpp')
#     output_classes_fw(streams['cpp_header'], intro, cfg)
    output_classes(streams['cpp_header'], intro, cfg)
    output_free_funs(streams['cpp_header'], intro, cfg)
    gencommon.finish_streams(cfg, streams)


def main(cfg,intro=None):
    if not intro:
        intro = introspect.introspect(cfg, True)
    gen_cpp_api(intro, cfg)


if __name__ == "__main__":
    cfg = jagbasecfg.cfg(True)
    main(cfg)
