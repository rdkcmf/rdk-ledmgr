#!/bin/bash
##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2016 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
#

#######################################
#
# Build Framework standard script for
#
# ledmgr component

# use -e to fail on any shell issue
# -e is the requirement from Build Framework
set -e


# default PATHs - use `man readlink` for more info
# the path to combined build
export RDK_PROJECT_ROOT_PATH=${RDK_PROJECT_ROOT_PATH-`readlink -m ..`}
export COMBINED_ROOT=$RDK_PROJECT_ROOT_PATH

# path to build script (this script)
export RDK_SCRIPTS_PATH=${RDK_SCRIPTS_PATH-`readlink -m $0 | xargs dirname`}

# path to components sources and target
export RDK_SOURCE_PATH=${RDK_SOURCE_PATH-$RDK_SCRIPTS_PATH}
export RDK_TARGET_PATH=${RDK_TARGET_PATH-$RDK_SOURCE_PATH}

# fsroot and toolchain (valid for all devices)
export RDK_FSROOT_PATH=${RDK_FSROOT_PATH-`readlink -m $RDK_PROJECT_ROOT_PATH/sdk/fsroot/ramdisk`}
export RDK_TOOLCHAIN_PATH=${RDK_TOOLCHAIN_PATH-`readlink -m $RDK_PROJECT_ROOT_PATH/sdk/toolchain/staging_dir`}


# default component name
export RDK_COMPONENT_NAME=${RDK_COMPONENT_NAME-`basename $RDK_SOURCE_PATH`}
export RDK_DIR=${RDK_PROJECT_ROOT_PATH}


# parse arguments
INITIAL_ARGS=$@

function usage()
{
    set +x
    echo "Usage: `basename $0` [-h|--help] [-v|--verbose] [action]"
    echo "    -h    --help                  : this help"
    echo "    -v    --verbose               : verbose output"
    echo
    echo "Supported actions:"
    echo "      configure, clean, build (DEFAULT), rebuild, install"
}

# options may be followed by one colon to indicate they have a required argument
if ! GETOPT=$(getopt -n "build.sh" -o hvk -l help,verbose -- "$@")
then
    usage
    exit 1
fi

eval set -- "$GETOPT"

while true; do
  case "$1" in
    -h | --help ) usage; exit 0 ;;
    -v | --verbose ) set -x ;;
    -- ) shift; break;;
    * ) break;;
  esac
  shift
done

ARGS=$@


# component-specific vars
export FSROOT=$RDK_FSROOT_PATH


# functional modules

function configure()
{
	pd=`pwd`
	cd ${RDK_SOURCE_PATH}
	aclocal
	libtoolize --automake
	autoheader
	automake --foreign --add-missing
	rm -f configure
	autoconf
    echo "  CONFIG_MODE = $CONFIG_MODE"

	configure_options=" "
	if [ "x$DEFAULT_HOST" != "x" ]; then
	    configure_options="--host $DEFAULT_HOST"
    fi

    configure_options="$configure_options --enable-shared --with-pic"
    generic_options="$configure_options"

    if [ "$RDK_PLATFORM_SOC" = "stm" ];then
        configure_options="$configure_options --with-libtool-sysroot=${RDK_FSROOT_PATH}"
        autoreconf --verbose --force --install
    fi
     
    if [ "$RDK_PLATFORM_SOC" = "stm" ];then
        ./configure --prefix=/usr $configure_options $extraenablers
    else
        ./configure --prefix=${RDK_FSROOT_PATH}/usr $configure_options $extraenablers
    fi

	cd $pd
}

function clean()
{
    pd=`pwd`
    dnames="${RDK_SOURCE_PATH}" 
    for dName in $dnames
    do
        cd $dName
 	if [ -f Makefile ]; then
    		make distclean
	fi
    	rm -f configure;
    	rm -rf aclocal.m4 autom4te.cache config.log config.status libtool
    	find . -iname "Makefile.in" -exec rm -f {} \; 
        if [ "$RDK_PLATFORM_SOC" = "stm" ];then
            find . -name Makefile | grep -v "aes" | xargs rm -rf
        else
    	    find . -iname "Makefile" -exec rm -f {} \; 
        fi
    	ls cfg/* | grep -v "Makefile.am" | xargs rm -f
    	cd $pd
    done
}

function build()
{
    cd ${RDK_SOURCE_PATH}
    make
}

function rebuild()
{
    clean
    configure
    build
}

function install()
{
    cd ${RDK_SOURCE_PATH}

    if [ "$RDK_PLATFORM_SOC" = "stm" ];then
       make install DESTDIR=${RDK_FSROOT_PATH}
    else
       make install
    fi
}


# run the logic

#these args are what left untouched after parse_args
HIT=false

for i in "$ARGS"; do
    case $i in
        configure)  HIT=true; configure ;;
        clean)      HIT=true; clean ;;
        build)      HIT=true; build ;;
        rebuild)    HIT=true; rebuild ;;
        install)    HIT=true; install ;;
        *)
            #skip unknown
        ;;
    esac
done

# if not HIT do build by default
if ! $HIT; then
  rebuild
fi
