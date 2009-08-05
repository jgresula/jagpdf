// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCELIST_H_JG914__
#define __RESOURCELIST_H_JG914__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <resources/interfaces/resourcehandle.h>
#include <core/generic/noncopyable.h>
#include <boost/ref.hpp>
#include <set>

namespace jag {
namespace pdf {

class FontDictionary;

class ResourceList
    : public noncopyable
{
    template<class T>
    struct ResourceTraits
    {
        typedef std::set<T> Container;
        typedef typename Container::const_iterator Iter;
        typedef std::pair<Iter,Iter> Range;
    };

    typedef boost::reference_wrapper<FontDictionary> FontDictionaryRef;
    friend bool operator<(FontDictionaryRef const& lhs,
                          FontDictionaryRef const& rhs);

    ResourceTraits<PatternHandle>::Container       m_patterns;
    ResourceTraits<ImageHandle>::Container           m_images;
    ResourceTraits<ColorSpaceHandle>::Container       m_color_spaces;
    ResourceTraits<GraphicsStateHandle>::Container m_graphics_states;
    ResourceTraits<FontDictionaryRef>::Container     m_fonts;
    ResourceTraits<ShadingHandle>::Container       m_shadings;

public: //types
    typedef ResourceTraits<PatternHandle>::Range PatternsRange;
    typedef ResourceTraits<ImageHandle>::Range ImagesRange;
    typedef ResourceTraits<ColorSpaceHandle>::Range ColorSpacesRange;
    typedef ResourceTraits<GraphicsStateHandle>::Range GraphicsStatesRange;
    typedef ResourceTraits<FontDictionaryRef>::Range FontsRange;
    typedef ResourceTraits<ShadingHandle>::Range ShadingsRange;

public: //resource access
    PatternsRange patterns() const;
    ImagesRange images() const;
    ColorSpacesRange color_spaces() const;
    GraphicsStatesRange graphics_states() const;
    FontsRange fonts() const;
    ShadingsRange shadings() const;

    void add_pattern(PatternHandle handle);
    void add_image(ImageHandle handle);
    void add_color_space(ColorSpaceHandle handle);
    void add_graphics_state(GraphicsStateHandle handle);
    void add_font(FontDictionary& handle);
    void add_shading(ShadingHandle sh);
    

public: //general
    void append(ResourceList const&);
    bool is_empty() const;
};

bool operator<(ResourceList::FontDictionaryRef const& lhs,
               ResourceList::FontDictionaryRef const& rhs);

}} //namespace jag::pdf


#endif //__RESOURCELIST_H_JG914__

