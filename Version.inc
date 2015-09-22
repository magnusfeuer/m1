# Used both by shell scripts and makefiles
#
# git(hub) server
#

CORE_GIT_REPO ?= https://github.com/magnusfeuer/m1_core.git
CORE_GIT_BRANCH ?= master
export CORE_GIT_SERVER
export CORE_GIT_BRANCH

APP_GIT_REPO ?= https://github.com/magnusfeuer/m1_app.git
APP_GIT_BRANCH ?= master
export APP_GIT_SERVER
export APP_GIT_BRANCH

# The version of self to stamp files and distros with.
VERSION=2.0.0
