#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import introspect
import jagbasecfg
import sig
import gencommon
from string import Template


g_callable_muster_err_code = """{
    try {
        ${try_body}
        return 0;
    } catch (${internal_exception_cls} const& exc) {
        ${c_catch_set_global_error};
        return exc.errcode();
    }
}

"""
g_callable_muster_ptr = """{
    try {
        ${try_body}
    } catch (${internal_exception_cls} const& exc) {
        ${c_catch_set_global_error};
        return 0;
    }
}

"""

g_callable_muster_other= """{
    try {
        ${pre}
        return static_cast<${return_type}>(${invocation});
    } catch (${internal_exception_cls} const& exc) {
        ${c_catch_set_global_error};
        return ${err_ret_value};
    }
}

"""

def c_prepare_args(callable_):
    args=[]
    body=[]
    # fix it here --vv--
    if callable_.is_memfun():
        cls = callable_.callable_object().gcc.parent.decl_string.lstrip('::')
        body.append('%s* this__(handle2ptr<%s>(hobj));' % (cls, cls))
    # --^^--
    cargs = callable_.args()
    for i, arg in enumerate(callable_.args()):
        if arg['category'] == 'interface':
            if arg['ref_counted']:
                fmtstr = 'boost::intrusive_ptr<%s> const& %s(handle2ptr<%s>(%s));'
            else:
                fmtstr = '%s* %s(handle2ptr<%s>(%s));'
            args.append('local__%d' % i)
            body.append(fmtstr\
                         % (cargs[i]['base_name'],
                             args[-1] ,
                             cargs[i]['base_name'],
                             cargs[i]['argname']))
        elif arg['category'] == 'enum':
            args.append('static_cast<%s>(%s)' % (cargs[i]['base_name'],
                                                   cargs[i]['argname']))
        else:
            args.append(cargs[i]['argname'])
    return args, body



def c_callable_body_returning_void(callable_, cfg):
    arglist, body = c_prepare_args(callable_)
    arglist = ', '.join(arglist)
    if callable_.is_memfun():
        body.append("this__->%s(%s);" % (callable_.callable_object().name(), arglist))
    else:
        body.append("%s(%s);" % (callable_.callable_object().name(), arglist))
    return { 'try_body' : "\n        ".join(body) }


def c_callable_body_returning_interface(callable_, cfg):
    arglist, body = c_prepare_args(callable_)
    arglist = ', '.join(arglist)
    result_ptr_type = callable_.return_type()['base_name'];
    if callable_.return_type()['ref_counted']:
        fmtstr = "boost::intrusive_ptr<%s> const& result__ = %s%s(%s);"
        fmt_ret = "return ptr2handle<%s>(result__.get());"
    else:
        fmtstr = "%s* result__ = %s%s(%s);"
        fmt_ret = "return ptr2handle<%s>(result__);"
    body.append(fmtstr %
                 (result_ptr_type,
                  callable_.is_memfun() and 'this__->' or '',
                  callable_.callable_object().name(),
                  arglist))
    opts = callable_.callable_object().opts
    result_ptr_handle = gencommon.c_handle_from_cls(callable_.return_type()['base'], opts)
    body.append(fmt_ret % result_ptr_handle)
    return { 'try_body' : "\n        ".join(body) }


def c_callable_body_returning_other(callable_, cfg):
    arglist, pre = c_prepare_args(callable_)
    pre = "\n        ".join(pre)
    arglist = ', '.join(arglist)
    return_type = gencommon.map_type(callable_.return_type(), 'c', cfg)
    invocation = "%s%s(%s)" % (callable_.is_memfun() and 'this__->' or '',
                                callable_.callable_object().name(),
                                arglist)
    if callable_.return_type()['category'] == 'const_pointer':
        err_ret_value = '0'
    else:
        err_ret_value = cfg['err_ret_value'](return_type)
    return locals()



def c_callable_body(stream, freefun, cfg):
    """writes body of a free fun"""
    body_disp = { 'interface' : g_callable_muster_ptr,
                  'void' : g_callable_muster_err_code,
                  'other' : g_callable_muster_other }
    args = []
    mi = freefun.metainfo()
    ret_cat = mi.return_type_category()
    subst_dict = globals()['c_callable_body_returning_'+ret_cat](mi, cfg)
    gencommon.update_dict(subst_dict, cfg, 'internal_exception_cls', 'c_catch_set_global_error')
    stream.write(Template(body_disp[ret_cat]).substitute(subst_dict))



def c_output_free_funs(header, impl, intro, cfg):
    """writes callables declarations/definitions"""
    s = sig.c_signature(cfg)
    funcs = [(s.get(f),f) for f in intro.exported_freefunctions()]
    funcs.sort()
    header.write('\n/* ==== free functions ==== */\n')
    for csig, free_fun in funcs:
        header.write("%s;\n" % csig)
        impl.write("%s\n" % csig)
        c_callable_body(impl, free_fun, cfg)



def c_output_class_handles(header, impl, intro, cfg):
    """writes callables declarations/definitions"""
    header.write('\n/* ==== handles ==== */\n')
    handles = [gencommon.c_handle_from_cls(cls,cfg) \
               for cls in intro.exported_classes()]
    handles.sort()
    for handle in handles:
        header.write(cfg['c_cls_handle_gen'] % handle)

def c_output_class_methods(header, impl, intro, cfg):
    """writes callables declarations/definitions"""
    s = sig.c_signature(cfg)
    header.write('\n/* ==== methods ===== */\n')
    methods = [(s.get(m), m) \
               for cls in intro.exported_classes()
               for m in cls.methods()]
    methods.sort()
    for csig, m in methods:
        csig = s.get(m)
        header.write("%s;\n" % csig)
        impl.write("%s\n" % csig)
        c_callable_body(impl, m, cfg)




def gen_c_api(intro, cfg):
    streams = gencommon.get_streams(cfg, 'c_header', 'c_impl')
    gencommon.output_enums(streams['c_header'], intro, cfg, 'c')
    c_output_class_handles(streams['c_header'], streams['c_impl'], intro, cfg)
    c_output_free_funs(streams['c_header'], streams['c_impl'], intro, cfg)
    c_output_class_methods(streams['c_header'], streams['c_impl'], intro, cfg)
    gencommon.finish_streams(cfg, streams)


def main(cfg, intro=None):
    if not intro:
        intro = introspect.introspect(cfg, True)
    gen_c_api(intro, cfg)



if __name__ == "__main__":
    cfg = jagbasecfg.cfg(True)
    main(cfg)
