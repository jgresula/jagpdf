// -*- mode: c++ -*-

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


namespace boost {
  template<class T> class shared_ptr
  {
    public:
      T * operator-> () const;
  };

  template<class T> class intrusive_ptr
  {
    public:
      T * operator-> () const;
  };
}


%define shared_ptr(T)
    %template( shared_ptr ## __LINE__ ) boost::shared_ptr<T>;
%enddef

