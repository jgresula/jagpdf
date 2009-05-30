// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "../systemfontmapping.h"
#include "../freetypeopenargs.h"
#include "../fontspecimpl.h"
#include <resources/typeman/typefaceimpl.h>
#include <resources/typeman/charencrecord.h>
#include <msg_resources.h>
#include <core/jstd/crt.h>
#include <core/generic/stringutils.h>
#include <core/generic/assert.h>
#include <core/generic/scopeguard.h>
#include <core/generic/refcountedimpl.h>

using namespace jag::jstd;

#include <windows.h>

namespace jag {
namespace resources {

//////////////////////////////////////////////////////////////////////////
unsigned long ft_stream_read_win_hfont(
      FT_Stream stream
    , unsigned long offset
    , unsigned char* buffer
    , unsigned long count
)
{
    JAG_PRECONDITION_MSG(stream->descriptor.pointer, "stream probably already closed");
    if (count)
    {
        HDC dc = (HDC)stream->descriptor.pointer;
        DWORD retval = ::GetFontData(dc, 0, offset, buffer, count);
        JAG_ASSERT(retval != GDI_ERROR);
        return retval;
    }
    else
    {
        stream->pos = offset;
        return 0;
    }
}

//////////////////////////////////////////////////////////////////////////
void ft_stream_close_win_hfont(FT_Stream stream)
{
    HDC dc = (HDC)stream->descriptor.pointer;
    HGDIOBJ font = ::GetCurrentObject(dc, OBJ_FONT);
    ::DeleteObject(font);
    ::ReleaseDC(0, dc);
    stream->descriptor.pointer = 0;
}


//////////////////////////////////////////////////////////////////////////
std::auto_ptr<FT_StreamRec> create_ft_stream(HDC dc)
{
    std::auto_ptr<FT_StreamRec> ft_stream(new FT_StreamRec);
    memset(ft_stream.get(), 0, sizeof(FT_StreamRec));
    ft_stream->size = ::GetFontData(dc, 0, 0, 0, 0);
    ft_stream->descriptor.pointer = dc;
    ft_stream->read = &ft_stream_read_win_hfont;
    ft_stream->close = &ft_stream_close_win_hfont;
    return ft_stream;
}

//////////////////////////////////////////////////////////////////////////
std::auto_ptr<TypefaceImpl> typeface_from_system(
    boost::shared_ptr<FT_LibraryRec_> ftlib
    , FontSpecImpl const* specimpl
    , CharEncodingRecord const& enc_rec
)
{
    JAG_PRECONDITION(!is_empty(specimpl->facename()));

    if (enc_rec.encoding == ENC_BUILTIN)
    {
        // this way we detect that no encoding was specified in the font
        // definition string
        throw exception_invalid_value(
            msg_enc_no_specified_sys_font_mapper()) << JAGLOC;
    }
    if (-1 == enc_rec.win_lfcharset)
    {
        throw exception_invalid_value(
            msg_enc_not_supported_by_sys_font_mapper()) << JAGLOC;
    }

    LOGFONT logfont;
    memset(&logfont, 0, sizeof(LOGFONT));
    logfont.lfItalic = static_cast<BYTE>(specimpl->italic());
    logfont.lfWeight = specimpl->bold() ? FW_BOLD : FW_NORMAL;
    logfont.lfCharSet = static_cast<BYTE>(enc_rec.win_lfcharset);
    strncpy(logfont.lfFaceName, specimpl->facename(), 31);

    HDC dc = ::GetDC(0);
    ScopeGuard dc_guard(MakeGuard(&::ReleaseDC, (HWND)0, dc));
    HFONT hfont = ::CreateFontIndirect(&logfont);
    ScopeGuard font_guard(MakeGuard(&::DeleteObject, hfont));
    ::SelectObject(dc, hfont);

    // check if the face names are the same as the mapper never fails
    // if it can't find the specified font it selects 'the closest one'
    char facename[LF_FACESIZE];
    ::GetTextFace(dc, LF_FACESIZE, facename);
    if (JAG_STRICMP(facename, specimpl->facename()))
        return std::auto_ptr<TypefaceImpl>();


    std::auto_ptr<FTOpenArgs> ft_args(new FTOpenArgs(create_ft_stream(dc)));
    std::auto_ptr<TypefaceImpl> typeface(new TypefaceImpl(ftlib, ft_args));
    // FT_Stream takes care of dc and font
    font_guard.Dismiss();
    dc_guard.Dismiss();

    return typeface;
}


}} // namespace jag::resources


