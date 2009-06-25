// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __COLOR_JG2141_H__
#define __COLOR_JG2141_H__

#include <resources/resourcebox/colorspacehelpers.h>

namespace jag {
namespace pdf {

/**
 * @brief Represents a color value.
 *
 * @todo depending on public interface change
 *       assertions in ctor to exceptions
 *
 * defaultly constructible, copyable, copy-constructible
 */
class Color
{
public: // construction
    /// invalid color
    Color() : m_type(CH0) {};
    /// gray, calgray
    explicit Color(Double ch1);
    /// rgb, cie, calrgb
    Color(Double ch1, Double ch2, Double ch3);
    /// cmyk
    Color(Double ch1, Double ch2, Double ch3, Double ch4);

    // colored pattern
    Color(PatternHandle ph);

    // uncolored patterns
    Color(PatternHandle ph, Double ch1);
    Color(PatternHandle ph, Double ch1, Double ch2, Double ch3);
    Color(PatternHandle ph, Double ch1, Double ch2, Double ch3, Double ch4);

public: // getters
    // would be good to have these getters checked
    Double channel1() const { return m_channel[0]; }
    Double channel2() const { return m_channel[1]; }
    Double channel3() const { return m_channel[2]; }
    Double channel4() const { return m_channel[3]; }
    unsigned index() const { return static_cast<unsigned>(m_channel[0]); }
    PatternHandle pattern() const { return m_pattern; }

    friend bool operator==(Color const&, Color const&);

private:
    enum { CH0, CH1, CH3, CH4, CH1_P, CH3_P, CH4_P, P0 };
    PatternHandle m_pattern;
    Double   m_channel[4];
    int m_type;
};


bool operator==(Color const&, Color const&);
inline bool operator!=(Color const& lhs, Color const& rhs)
{
    return !(lhs==rhs);
}


}} // namespace jag::pdf

#endif // __COLOR_JG2141_H__
/** EOF @file */
