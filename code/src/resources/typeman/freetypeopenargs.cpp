// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "freetypeopenargs.h"
#include <core/generic/assert.h>
#include <core/jstd/fileso.h>
#include <core/errlib/errlib.h>
#include <msg_resources.h>

using namespace jag::jstd;

namespace jag {
namespace resources {



//////////////////////////////////////////////////////////////////////////
FTOpenArgs::FTOpenArgs(std::auto_ptr<FT_StreamRec> stream)
    : m_num_records(1)
{
    insert(0, stream);
}



//////////////////////////////////////////////////////////////////////////
FTOpenArgs::FTOpenArgs(Char const* filename)
    : m_num_records(1)
{
    insert(0, filename);
}



void FTOpenArgs::insert(int index, Char const* filename)
{
    JAG_PRECONDITION(0==index || 1==index);

    if (!is_file(filename))
        throw exception_io_error(msg_font_file_not_found())
            << io_object_info(filename)
            << JAGLOC;


    m_filename[index].assign(filename);
    memset(m_args+index, 0, sizeof(FT_Open_Args));
    m_args[index].flags = FT_OPEN_PATHNAME;
    m_args[index].pathname = const_cast<char*>(m_filename[index].c_str());
    m_data_size[index] = static_cast<unsigned int>(
        file_size(m_filename[index].c_str()));
}



void FTOpenArgs::insert(int index, std::auto_ptr<FT_StreamRec> stream)
{
    JAG_PRECONDITION(0==index || 1==index);

    m_stream[index].reset(stream.release());
    memset(m_args+index, 0, sizeof(FT_Open_Args));
    m_args[index].flags = FT_OPEN_STREAM;
    m_args[index].stream = m_stream[index].get();
    m_data_size[index] = m_args[index].stream->size;

}



void FTOpenArgs::attach(Char const* filename)
{
    JAG_PRECONDITION(m_num_records == 1);
    insert(1, filename);
    m_num_records = 2;
}



FT_Open_Args* FTOpenArgs::get_args(int index)
{
    JAG_PRECONDITION(index >=0 && index < m_num_records);
    return m_args + index;
}



int FTOpenArgs::data_size(int index) const
{
    JAG_PRECONDITION(index >=0 && index < m_num_records);
    return m_data_size[index];
}

char const* FTOpenArgs::filename(int index) const
{
    JAG_PRECONDITION(index >=0 && index < m_num_records);
    return m_filename[index].c_str();
}


}} // namespace jag::resources
