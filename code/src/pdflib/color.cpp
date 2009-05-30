// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "color.h"
#include <resources/resourcebox/colorspacehelpers.h>

namespace jag {
namespace pdf {

// helper checking macros


////////////////////////////////////////////
// class Color
////////////////////////////////////////////

Color::Color(Double ch1)
{
    m_channel[0] = ch1;
}



Color::Color(Double ch1, Double ch2, Double ch3)
{
    m_channel[0] = ch1;
    m_channel[1] = ch2;
    m_channel[2] = ch3;
}



Color::Color(Double ch1, Double ch2, Double ch3, Double ch4)
{
    m_channel[0] = ch1;
    m_channel[1] = ch2;
    m_channel[2] = ch3;
    m_channel[3] = ch4;
}



Color::Color(PatternHandle ph, Double ch1)
    : m_pattern(ph)
{
    m_channel[0] = ch1;
}



Color::Color(PatternHandle ph, Double ch1, Double ch2, Double ch3)
    : m_pattern(ph)
{
    m_channel[0] = ch1;
    m_channel[1] = ch2;
    m_channel[2] = ch3;
}



Color::Color(PatternHandle ph, Double ch1, Double ch2, Double ch3, Double ch4)
    : m_pattern(ph)
{
    m_channel[0] = ch1;
    m_channel[1] = ch2;
    m_channel[2] = ch3;
    m_channel[3] = ch4;
}



Color::Color(PatternHandle ph)
    : m_pattern(ph)
{
}



}} // namespace jag

/** EOF @file */
