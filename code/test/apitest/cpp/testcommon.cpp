// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "testcommon.h"
#include <cassert>
#include <string>
#include <sstream>
#include <stdexcept>


static char const* g_output_dir=0;

using namespace jag;
//
//
//
#ifndef JAG_DO_NOT_USE_EXCEPTIONS
int test_runner(void (*test)(int, char**), int argc, char **argv)
{
    int result=0;

    try
    {
        test(argc, argv);
    }
    catch(pdf::Exception const& exc)
    {
        std::cerr << "ERROR(" << exc.code() << "):\n" << exc.what() << '\n';
        std::cerr << ">> cpp client failed\n";
        result=1;
    }
    catch(std::exception const& exc)
    {
        std::cerr << "ERROR:" << exc.what() << '\n';
        std::cerr << ">> cpp client failed\n";
        result=1;
    }

    result += boost::report_errors();
    return result;
}
#endif


char const* get_output_dir()
{
    assert(g_output_dir);
    return g_output_dir;
}

void register_command_line(int argc, char **argv)
{
   if (argc != 2)
        throw std::runtime_error("no output directory specified");

   g_output_dir=argv[1];
}


pdf::Document create_doc(char const* fname, pdf::Profile* cfg)
{

    std::string out_file(get_output_dir());
    out_file += '/' ;
    out_file += fname ;

    if (!cfg)
    {
        pdf::Profile local_cfg(pdf::create_profile());
        local_cfg.set("doc.compressed", "0");
        return pdf::create_file(out_file.c_str(), local_cfg);
    }

    return pdf::create_file(out_file.c_str(), *cfg);
}


///////////////////////////////////////////////////////////////////////////
EasyFontBase::EasyFontBase(pdf::Document* writer)
    : m_writer(writer)
{}

void EasyFontBase::set_writer(pdf::Document& writer)
{
    m_writer = &writer;
    m_fonts.clear();
}

pdf::Font EasyFontBase::operator()(int ptsize)
{
    assert(m_writer);
    Map::iterator it = m_fonts.find(ptsize);
    if (it == m_fonts.end())
    {
        pdf::Font font = m_writer->font_load(
            get_font_spec(ptsize).c_str());

        assert(font);
        it = m_fonts.insert(std::make_pair(ptsize,font)).first;
    }

    return it->second;
}


//
//
//
std::string EasyFontTTF::get_font_spec(int ptsize) const
{
    std::ostringstream fspec;
    fspec
        << "enc=windows-1252; size="
        << ptsize << "; file="
        <<  getenv("JAG_TEST_RESOURCES_DIR")
        << "/fonts/DejaVuSans.ttf";
    return fspec.str();
}

//
//
//
std::string EasyFont::get_font_spec(int ptsize) const
{
    std::ostringstream fspec;
    fspec << "standard;name=Helvetica;size=" << ptsize;
    return fspec.str();
}



/** EOF @file */
