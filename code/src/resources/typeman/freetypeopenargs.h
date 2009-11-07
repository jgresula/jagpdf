// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FREETYPEOPENARGS_H_JAG_1336__
#define __FREETYPEOPENARGS_H_JAG_1336__

#include <core/generic/noncopyable.h>
#include <interfaces/stdtypes.h>
#include <boost/scoped_ptr.hpp>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace jag { namespace resources
{


class FTOpenArgs
    : public noncopyable
{
public:
    /// construction from FT_Stream
    FTOpenArgs(std::auto_ptr<FT_StreamRec> stream);

    /// construction from filename
    FTOpenArgs(Char const* filename);
    void attach(Char const* filename);

public:
    int num_records() const { return m_num_records; }
    FT_Open_Args* get_args(int index);
    int data_size(int index) const;
    char const* filename(int index) const;

private:
    void insert(int index, Char const* filename);
    void insert(int index, std::auto_ptr<FT_StreamRec> stream);

private:
    int m_num_records;
    FT_Open_Args m_args[2];
    boost::scoped_ptr<FT_StreamRec> m_stream[2];
    std::string m_filename[2];
    unsigned int m_data_size[2];
};


}} // namespace jag::resources

#endif //__FREETYPEOPENARGS_H_JAG_1336__
