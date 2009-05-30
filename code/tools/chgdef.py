#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import re

class Bunch:
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

def _rex( text ):
    return re.compile( text, re.MULTILINE | re.DOTALL )



# -- cpp head --------------------
c_head_txt="""// $(Copyright)
// $(License)
//
// \x24Id\x24
"""
c_head_rex="""^// \$\(Copyright\)
// \$\(License\)
//
\n]*\$$
"""
c_head_repl='(?:^//.*?\n)+|^/\*.*?\*/\n+'


# -- cpp tail --------------------
c_tail_txt="/** EOF @file */\n"
c_tail_rex="^/\*\* EOF @file \*/\n+\Z"
c_tail_repl=""




# -- python head --------------------
py_head_txt="""#!/usr/bin/env python
#
# $(Copyright)
# $(License)
#
# \x24Id\x24
"""
py_head_rex="""#!/usr/bin/env python
#
# \$\(Copyright\)
# \$\(License\)
#
\n]*\$$
"""
py_head_repl='(?:^#.*?\n)+'


cfg = {}
cfg['*.cpp'] = Bunch( head_t=c_head_txt, head_r=_rex(c_head_rex),\
                      tail_t=c_tail_txt,  tail_r = _rex(c_tail_rex),\
                      head_repl_r=_rex(c_head_repl), tail_repl_r=_rex(c_tail_repl) )
cfg['*.h']=cfg['*.cpp']

cfg['*.py'] = Bunch( head_t=py_head_txt, head_r=_rex(py_head_rex),\
                     tail_t=None,  tail_r = None,\
                     head_repl_r=_rex(py_head_repl), tail_repl_r=None )



























