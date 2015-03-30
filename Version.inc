# Used both by shell scripts and makefiles
#
# git(hub) server
#

#CORE_GIT_SERVER ?= https://github.com/magnusfeuer
CORE_GIT_REPO ?= git@github.com:magnusfeuer/m1_core
CORE_GIT_BRANCH ?= master
export CORE_GIT_SERVER
export CORE_GIT_BRANCH

#APP_GIT_SERVER ?= https://github.com/magnusfeuer
APP_GIT_REPO ?= git@github.com:magnusfeuer/m1_app
APP_GIT_BRANCH ?= master
export APP_GIT_SERVER
export APP_GIT_BRANCH

# The version of self to stamp files and distros with.
VERSION=2.0.0
