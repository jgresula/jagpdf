// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "resourcelist.h"
#include <core/generic/assert.h>
#include "fontdictionary.h"

namespace jag {
namespace pdf {



//
//
//
void ResourceList::add_shading(ShadingHandle sh)
{
    m_shadings.insert(sh);
}

//
//
//
ResourceList::ShadingsRange ResourceList::shadings() const
{
    return ShadingsRange(m_shadings.begin(), m_shadings.end());
}


//////////////////////////////////////////////////////////////////////////
ResourceList::PatternsRange ResourceList::patterns() const
{
    return PatternsRange(m_patterns.begin(), m_patterns.end());
}

//////////////////////////////////////////////////////////////////////////
void ResourceList::add_pattern(PatternHandle handle)
{
    m_patterns.insert(handle);
}


//////////////////////////////////////////////////////////////////////////
ResourceList::ImagesRange ResourceList::images() const
{
    return ImagesRange(m_images.begin(), m_images.end());
}

//////////////////////////////////////////////////////////////////////////
void ResourceList::add_image(ImageHandle handle)
{
    m_images.insert(handle);
}


//////////////////////////////////////////////////////////////////////////
ResourceList::ColorSpacesRange ResourceList::color_spaces() const
{
    return ColorSpacesRange(m_color_spaces.begin(), m_color_spaces.end());
}

//////////////////////////////////////////////////////////////////////////
void ResourceList::add_color_space(ColorSpaceHandle handle)
{
    m_color_spaces.insert(handle);
}

//////////////////////////////////////////////////////////////////////////
ResourceList::GraphicsStatesRange ResourceList::graphics_states() const
{
    return GraphicsStatesRange(m_graphics_states.begin(), m_graphics_states.end());
}

//////////////////////////////////////////////////////////////////////////
void ResourceList::add_graphics_state(GraphicsStateHandle handle)
{
    m_graphics_states.insert(handle);
}

//////////////////////////////////////////////////////////////////////////
ResourceList::FontsRange ResourceList::fonts() const
{
    return FontsRange(m_fonts.begin(), m_fonts.end());
}

//////////////////////////////////////////////////////////////////////////
void ResourceList::add_font(FontDictionary& fdict)
{
    m_fonts.insert(FontDictionaryRef(fdict));
}

//////////////////////////////////////////////////////////////////////////
void ResourceList::append(ResourceList const&)
{
    JAG_TBD;
}

bool ResourceList::is_empty() const
{
    return
           m_patterns.empty()
        && m_color_spaces.empty()
        && m_images.empty()
        && m_graphics_states.empty()
        && m_fonts.empty()
        && m_shadings.empty()
    ;
}

//
//
//
bool operator<(ResourceList::FontDictionaryRef const& lhs,
               ResourceList::FontDictionaryRef const& rhs)
{
    return lhs.get().id() < rhs.get().id();
}


}} //namespace jag::pdf
