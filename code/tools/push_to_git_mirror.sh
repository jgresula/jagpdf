#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

# push code from svn to the git mirror only if the nighly release tests suceeded

set -e
#set -x

SVN_URL=svn://jarda-home/trunk/jagbase
REV=`svn info $SVN_URL | grep 'Last Changed Rev:' | cut -d ' ' -f 4`
DB=/mnt/win/c/home/jarda/trac/db/trac.db
SQL="select rev, status, config from bitten_build 
         where rev=$REV and config=\"nightly_release_cmake\" 
         order by rev DESC limit 2;
"

SVN_IGNORE_PATHS=--ignore-paths=^trunk/jagbase/external

if [ ! -f $DB ]; then
    echo "ERROR: $DB not found."
    exit 2
fi

SQL_LINES=`sqlite3 $DB "$SQL" | grep '|S|' | wc -l`
if [ "2" == "$SQL_LINES" ]; then
    cd /home/jarda/code/github-mirrors/jagbase.git
    git checkout master
    git svn rebase "$SVN_IGNORE_PATHS"
    git push origin master
    git checkout idle
else
    echo "Nightly tests failed, mirror won't be updated."
    exit 13
fi
