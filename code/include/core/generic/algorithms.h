// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#ifndef __ALGORITHMS_JG1020_H__
# define __ALGORITHMS_JG1020_H__

namespace jag {


/// copy_if was somehow dropped from the standard (see TC++PL 18.6.1)
template <class In, class Out, class Pred>
Out copy_if(In first, In last, Out res, Pred p)
{
    while (first != last)
    {
        if (p(*first))
            *res++ = *first;
        ++first;
    }

    return res;
}


} // namespace jag

#endif // __ALGORITHMS_JG1020_H__
/** EOF @file */
