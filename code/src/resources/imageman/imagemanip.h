// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEMANIP_JG1335_H__
# define __IMAGEMANIP_JG1335_H__

namespace jag {
// fwd
class ISeqStreamInput;
class ISeqStreamOutput;

namespace resources {

/**
 * @brief Downsamples a sequence of 16-bit samples to 8 bits.
 *
 * 16 bit samples are assumed to be big-endian ordered.
 *
 * @param in    stream with 16-bit samples
 * @param out   stream where to write 8-bit samples
 */
void downsample_big16_to_8(ISeqStreamInput& in, ISeqStreamOutput& out);


}} // namespace jag::resources

#endif // __IMAGEMANIP_JG1335_H__
/** EOF @file */
