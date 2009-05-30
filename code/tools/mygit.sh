#!/bin/bash

# Copyright (c) 2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

function latest_svn_version
{
    svn info $SVN_URL | grep 'Last Changed Rev:' | cut -d ' ' -f 4
}

# test configuration
SVN_URL=svn://jarda-home/trunk/jproof
SVN_IGNORE_PATHS=--ignore-paths=\"^.*/ignore_dir\"
SVN_REV=-r1980:HEAD
GIT_MIRROR=git@githubmirror:jgresula/jproof.git
LOCAL_GIT_MIRROR=~/tmp/repo.git


# local configuration
SVN_URL=svn://jarda-home/trunk/jagbase
SVN_IGNORE_PATHS=--ignore-paths=^trunk/jagbase/external
SVN_REV=-r`latest_svn_version`
GIT_MIRROR=/home/jarda/code/github-mirrors/github-test.git
LOCAL_GIT_MIRROR=/home/jarda/code/github-mirrors/jagbase-test.git

# github configuration
SVN_URL=svn://jarda-home/trunk/jagbase
SVN_IGNORE_PATHS=--ignore-paths=^trunk/jagbase/external
SVN_REV=-r`latest_svn_version`
GIT_MIRROR=git@githubmirror:jgresula/jagpdf.git
LOCAL_GIT_MIRROR=/home/jarda/code/github-mirrors/jagbase.git

function help
{
echo "
  --help               this screen
  --init-working       initializes a local working dir
  --init-svn-mirror    initializes a local SVN mirror
  --push-to-mirror     pushes SVN head to the public mirror
  --resurrect-mirror   resurrects lost local SVN mirror
  --pull-fork <clone>  pulls a git clone and commits it to SVN
"
exit 1
}


INIT_SVN_MIRROR=
PUSH_TO_MIRROR=
RESURRECT_MIRROR=
PULL_FORK=
INIT_WORKING=

function check_empty_dir
{
    if [ -d ".git" ]; then
        echo "git repository already exists"
        exit 1
    fi
}

while true
do
    if [ "$1" == "--init-svn-mirror" ]; then
        INIT_SVN_MIRROR=1; shift
    elif [ "$1" == "--push-to-mirror" ]; then
        PUSH_TO_MIRROR=1; shift
    elif [ "$1" == "--resurrect-mirror" ]; then
        RESURRECT_MIRROR=1; shift
    elif [ "$1" == "--init-working" ]; then
        INIT_WORKING=1; shift
    elif [ "$1" == "--pull-fork" ]; then
        shift; PULL_FORK="$1"; shift
    elif [ "$1" == "--help" ]; then
        help;
    else
        break
    fi
done


#
# Establishes a working directory. Creates a git directory mirroring SVN HEAD.
#
#    GIT command           SVN command
#  ------------------------------------
#   git svn rebase         svn update
#   git svn dcommit        svn commit
#   git diff trunk         svn diff
#
if [ -n "$INIT_WORKING" ]; then
    check_empty_dir
    echo git init
    echo git config receive.denyCurrentBranch refuse
    echo git svn init -T $SVN_URL
    echo git svn fetch $SVN_REV
fi


#
# Establishes a git repository that mirrors SVN upstream and pushes $SVN_REV to
# $GIT_MIRROR. The mirror is held in 'master', but the current branch *must* be
# 'idle' to avoid pushing to a current branch.
#
if [ -n "$INIT_SVN_MIRROR" ]; then
    # http://www.fnokd.com/2008/08/20/mirroring-svn-repository-to-github/
    check_empty_dir
    echo git init
    echo git config receive.denyCurrentBranch refuse
    echo git svn init -T $SVN_URL "$SVN_IGNORE_PATHS"
    echo git svn fetch $SVN_REV "$SVN_IGNORE_PATHS"
    echo git remote add origin $GIT_MIRROR
    echo git push origin master
    echo git checkout -b idle
fi

#
# Pushes SVN HEAD to $GIT_MIRROR.
#
if [ -n "$PUSH_TO_MIRROR" ]; then
    git checkout master
    # ~ svn update
    git svn rebase "$SVN_IGNORE_PATHS"
    git push origin master
    git checkout idle
fi

#
# Resurrects the local mirror from $GIT_MIRROR. Useful when the local git mirror
# is lost.
#
if [ -n "$RESURRECT_MIRROR" ]; then
    check_empty_dir
    echo "--- cloning $GIT_MIRROR to get revision range"
    TMP_DIR=mygit_tmp_$$
    git clone $GIT_MIRROR $TMP_DIR
    cd $TMP_DIR
    REV_BEGIN=`git log | grep -Eo 'git-svn-id:.+@[0-9]+' | head -n 1 | cut -d '@' -f 2`
    REV_END=`git log | grep -Eo 'git-svn-id:.+@[0-9]+' | tail -n 1 | cut -d '@' -f 2`
    cd -
    rm -rf $TMP_DIR
    echo "-- found range: $REV_END:$REV_BEGIN"
    
    echo git init
    echo git config receive.denyCurrentBranch refuse
    echo git svn init -T $SVN_URL "$SVN_IGNORE_PATHS"
    echo git remote add origin $GIT_MIRROR
    echo git svn fetch -r$REV_END:$REV_BEGIN "$SVN_IGNORE_PATHS"
    echo git fetch origin
    # the following command would sync the local svn mirror with svn upstream
    #echo git svn rebase "$SVN_IGNORE_PATHS"
    echo git push --dry-run origin master
    echo git push origin master
    echo git checkout -b idle
fi


#
# Pulls from a git clone. The clone is merged to the local git mirror and then
# comitted to SVN head.
#
# Do not push to $GIT_MIRROR immediatelly, wait for passed tests.
#
if [ -n "$PULL_FORK" ]; then
    check_empty_dir
    echo git init
    echo git remote add origin $LOCAL_GIT_MIRROR
    echo git remote add fork $PULL_FORK
    echo git fetch origin
    echo git fetch fork
    echo git merge fork/master
    echo git push --dry-run origin
    echo "echo ---- in mirror repo"
    echo cd $LOCAL_GIT_MIRROR
    echo git svn rebase
    echo git svn dcommit
    echo git push --dry-run origin master
fi


