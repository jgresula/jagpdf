#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import sys
import string
import getopt


# rc_content = '''
# #include "winres.h"
#
# VS_VERSION_INFO VERSIONINFO
# FILEVERSION ${major}, ${minor}, ${patch}, 0
# PRODUCTVERSION ${major}, ${minor}, ${patch}, 0
#  FILEFLAGSMASK 0x3fL
# #ifdef _DEBUG
#  FILEFLAGS 0x9L
# #else
#  FILEFLAGS 0x8L
# #endif
#  FILEOS 0x4L
#  FILETYPE 0x1L
#  FILESUBTYPE 0x0L
# BEGIN
#     BLOCK "StringFileInfo"
#     BEGIN
#         BLOCK "040904b0"
#         BEGIN
#             VALUE "Comments", "Modified by BZCToOn's"
#             VALUE "CompanyName", "Syntheretix"
#             VALUE "FileDescription", "rcversion MFC Application"
#             VALUE "FileVersion", "${major}, ${minor}, ${patch}, 0"
#             VALUE "InternalName", "rcversion"
#             VALUE "LegalCopyright", "Copyleft (C) Bzc ToOn'S 2002"
#             VALUE "OriginalFilename", "rcversion.EXE"
#             VALUE "PrivateBuild", "RCVERSION-20030212_100"
#             VALUE "ProductName", "rcversion Application"
#             VALUE "ProductVersion", "${major}, ${minor}, ${patch}, 0"
#         END
#     END
#     BLOCK "VarFileInfo"
#     BEGIN
#         VALUE "Translation", 0x409, 1200
#     END
# END
# '''




rc_content = '''
#include "winresrc.h"

VS_VERSION_INFO VERSIONINFO
FILEVERSION ${major}, ${minor}, ${patch}
PRODUCTVERSION ${major}, ${minor}, ${patch}
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName", "Jaroslav Gre\xb9ula"
            VALUE "FileVersion", "${major}, ${minor}, ${patch}"
            VALUE "LegalCopyright", "Copyright (C) 2005-2009 Jaroslav Gre\xb9ula"
            VALUE "ProductName", "JagPDF library, http://jagpdf.org"
            VALUE "FileDescription", "${description}"
            VALUE "ProductVersion", "${major}, ${minor}, ${patch}"
        END
    END
END
'''

def get_cfg():
    cfg = dict( major=None, minor=None, patch=None, svnrev=0 )
    try:
        opts, args = getopt.getopt( sys.argv[1:], "", [ "major=",
                                                        "minor=",
                                                        "patch=",
                                                        "svnrev=",
                                                        "description="] )
    except getopt.GetoptError, exc:
        print exc
        sys.exit(2)

    for opt, val in opts:
        cfg[opt[2:]]=val
        assert opt not in ['major', 'minor', 'patch'] or isinstance(val,int)

#    print args, cfg
    return args[0], cfg


def main():
    outfile, cfg = get_cfg()
    rc = string.Template(rc_content).substitute(cfg)
    urc = unicode( rc, 'iso-8859-2' )
    open( outfile, 'wb' ).write( urc.encode("utf-16") )


if __name__ == "__main__":
    main()
