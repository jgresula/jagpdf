// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "color.h"
#include <core/generic/floatpointtools.h>
#include <resources/resourcebox/colorspacehelpers.h>

namespace jag {
namespace pdf {

// helper checking macros


////////////////////////////////////////////
// class Color
////////////////////////////////////////////

Color::Color(Double ch1)
    : m_type(CH1)
{
    m_channel[0] = ch1;
}



Color::Color(Double ch1, Double ch2, Double ch3)
    : m_type(CH3)
{
    m_channel[0] = ch1;
    m_channel[1] = ch2;
    m_channel[2] = ch3;
}



Color::Color(Double ch1, Double ch2, Double ch3, Double ch4)
    : m_type(CH4)
{
    m_channel[0] = ch1;
    m_channel[1] = ch2;
    m_channel[2] = ch3;
    m_channel[3] = ch4;
}



Color::Color(PatternHandle ph, Double ch1)
    : m_pattern(ph)
    , m_type(CH1)
{
    m_channel[0] = ch1;
}



Color::Color(PatternHandle ph, Double ch1, Double ch2, Double ch3)
    : m_pattern(ph)
    , m_type(CH3)
{
    m_channel[0] = ch1;
    m_channel[1] = ch2;
    m_channel[2] = ch3;
}



Color::Color(PatternHandle ph, Double ch1, Double ch2, Double ch3, Double ch4)
    : m_pattern(ph)
    , m_type(CH4)
{
    m_channel[0] = ch1;
    m_channel[1] = ch2;
    m_channel[2] = ch3;
    m_channel[3] = ch4;
}



Color::Color(PatternHandle ph)
    : m_pattern(ph)
    , m_type(P0)
{
}

bool operator==(Color const& lhs, Color const& rhs)
{
    if (lhs.m_type != rhs.m_type || lhs.m_pattern != rhs.m_pattern)
        return false;

    switch(lhs.m_type)
    {
    case Color::P0:
    case Color::CH0:
        return true;

    case Color::CH4:
        if (!equal_doubles(lhs.m_channel[3], rhs.m_channel[3]))
            return false;

    case Color::CH3:
        if (!equal_doubles(lhs.m_channel[2], rhs.m_channel[2]) ||
            !equal_doubles(lhs.m_channel[1], rhs.m_channel[1]))
        {
            return false;
        }

    case Color::CH1:
        if (!equal_doubles(lhs.m_channel[0], rhs.m_channel[0]))
            return false;
    }

    return true;
}



}} // namespace jag

/** EOF @file */
