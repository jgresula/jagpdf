// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TRUETYPETABLE_H_JAG_1121__
#define __TRUETYPETABLE_H_JAG_1121__

#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

namespace jag {
namespace resources {

/// true type specific functionality
namespace truetype {

#ifdef JAGAPI_MSV
#    pragma warning(push)
#    pragma warning(disable:4127)
#endif

struct PostscriptTable
{
    typedef TT_Postscript table_struct;
    static const FT_Sfnt_Tag ft_sfnt_id = ft_sfnt_post;
    static const bool is_required = true;
    static char const* name() { return "post"; }
};

struct OS2Table
{
    typedef TT_OS2 table_struct;
    static const FT_Sfnt_Tag ft_sfnt_id = ft_sfnt_os2;
    static const bool is_required = true;
    static char const* name() { return "os2"; }
};

struct HorizontalHeader
{
    typedef TT_HoriHeader table_struct;
    static const FT_Sfnt_Tag ft_sfnt_id = ft_sfnt_hhea;
    static const bool is_required = true;
    static char const* name() { return "hhea"; }
};

struct PCLTTable
{
    typedef TT_PCLT table_struct;
    static const FT_Sfnt_Tag ft_sfnt_id = ft_sfnt_pclt;
    static const bool is_required = false;
    static char const* name() { return "pclt"; }
};

//////////////////////////////////////////////////////////////////////////
template<class TableTraits>
class Table
{
public:
    Table(FT_Face face, bool throw_if_required_not_found = true)
        : m_table(0)
    {
        m_table = reinterpret_cast<typename TableTraits::table_struct*>(
            FT_Get_Sfnt_Table(face, TableTraits::ft_sfnt_id));

        if (!m_table && TableTraits::is_required && throw_if_required_not_found)
            throw exception_invalid_input(msg_tt_table_not_found(TableTraits::name())) << JAGLOC;
    }


    bool exists() const
    {
        return m_table ? true : false;
    }


    typename TableTraits::table_struct* operator->() const
    {
        return m_table;
    }

private:
    typename TableTraits::table_struct* m_table;

};

#ifdef JAGAPI_MSV
#    pragma warning(pop)
#endif



}}} //namespace jag:resource::truetype

#endif //__TRUETYPETABLE_H_JAG_1121__

