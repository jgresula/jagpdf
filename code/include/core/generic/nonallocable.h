// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __NONALLOCABLE_H_JAG_2200__
#define __NONALLOCABLE_H_JAG_2200__

namespace jag
{

/// Prevents derivee from being allocated on the heap.
class nonallocable
{
    // standard allocation
    void *operator new(size_t size);
    void operator delete(void *ptr);

    // array allocation
    void *operator new[](size_t);
    void operator delete [](void *, size_t);

   void* operator new(size_t, nonallocable&);
   void  operator delete(void* ptr, nonallocable&);
};

} //namespace jag

#endif //__NONALLOCABLE_H_JAG_2200__

