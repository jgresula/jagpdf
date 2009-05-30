// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef TESTCOMMON_JG1414_H__
#define TESTCOMMON_JG1414_H__

#include <jagpdf/api.h>
// #include <boost/noncopyable.hpp>
// #include <boost/config.hpp>
#include "my_boost/lightweight_test.hpp"

#include <cassert>
#include <sstream>
#include <map>
#include <stdexcept>

jag::pdf::Document create_doc(char const* fname, jag::pdf::Profile* cfg=0);
void register_command_line(int argc, char **argv);
int test_runner(void (*test)(int, char**), int argc, char **argv);

class EasyFontBase
{
    jag::pdf::Document*               m_writer;
    typedef std::map<int,jag::pdf::Font> Map;
    Map m_fonts;

public:
    explicit EasyFontBase(jag::pdf::Document* writer = 0);
    void set_writer(jag::pdf::Document& writer);
    jag::pdf::Font operator()(int ptsize = 12);

protected:
    virtual std::string get_font_spec(int ptsize) const = 0;
    virtual ~EasyFontBase() {}
};

//
//
//
class EasyFontTTF
    : public EasyFontBase
{
protected:
    std::string get_font_spec(int ptsize) const;
};


//
//
//
class EasyFont
    : public EasyFontBase
{
protected:
    std::string get_font_spec(int ptsize) const;
};



//
//
//
class StrFmt
{
private: // not copyable
    StrFmt(StrFmt const&) {}
    StrFmt& operator==(StrFmt const&) {}

public:
    StrFmt() : m_done(false) {}

    template <class T>
    StrFmt& operator<<(const T& t) {
        assert(!m_done);
        m_s << t;
        return *this;
    }

    operator char const*() {
        if (!m_done)
        {
            m_str = m_s.str();
            m_done = true;
        }
        return m_str.c_str();
    }

private:
    std::ostringstream m_s;
    std::string        m_str;
    bool               m_done;
};


//
//
//
class StreamNoop
    : public jag::pdf::StreamOut
{
public:
    jag::pdf::Int write(void const*, jag::pdf::ULong) { return 0; }
    jag::pdf::Int close() { return 0; }
};



//
//
//
class StreamFile
    : public jag::pdf::StreamOut
{
    FILE *m_f;
public:
    StreamFile(char const* file)
    {
        m_f = fopen(file, "wb");
        if (!m_f)
            throw std::runtime_error("cannot open output file");
    }

    jag::pdf::Int write(void const* data, jag::pdf::ULong size) {
        if (size != fwrite(data, 1, size, m_f))
            return 1;

        return 0;
    }

    jag::pdf::Int close() {
        if (fclose(m_f))
            return 1;

        return 0;
    }
};




#define JAG_MUST_THROW(exp)                                \
    try {                                                    \
        exp;                                                 \
        BOOST_ERROR("Exception expected");                 \
    }                                                       \
    catch(pdf::Exception&) {}


template<class T>
void ignore_unused(T const&){}

#endif // TESTCOMMON_JG1414_H__
/** EOF @file */
