# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import pstats
s = pstats.Stats( 'profile_result' )
s.sort_stats( 'cumulative' )
s.print_stats( .1 )
# s.print_stats( .1, '__getattr__' )
# s.print_callers( .1, '__getattr__' )
# restr = 'find_class'
# s.print_callers( .1, restr )
# s.print_callees( .1, restr )
