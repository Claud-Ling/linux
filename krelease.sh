#!/bin/bash
#linux kernel release script

usage(){
	cat <<-EOF >& 2

        usage: ./${0##*/} target [OPTION]...
          i.e. "./${0##*/} uImage -a" for android kernel release
          or   "./${0##*/} uImage" for regular linux kernel release

          target           - build target, e.g. [uImage|vmlinux.bin|xxx]
          OPTION
            -u, --user #name    specify user name, default use current user name
            -H, --host #name    specify host name, default use current host name
            -v, --version #no   specify build version number
            -a, --android       mkrelease for android, override option 'u' and 'h'
            -h, --help          display this help and exit

	EOF
        exit 0
}

parseparam(){
	local android_mode=no
        local TEMP=`getopt -o ahu:H:v: --long user:,host:,version:,help,android \
                -n 'example.bash' -- "$@"`
        eval set -- "$TEMP"
    
        while true ; do
          case "$1" in
            -u|--user) echo "user '$2'" ; BUILD_USER="$2"; shift 2 ;;
            -H|--host) echo "host '$2'" ; BUILD_HOST="$2"; shift 2 ;;
            -v|--version) echo "version '$2'" ; BUILD_VERSION="$2"; shift 2 ;;
            -a|--android) echo "android" ; android_mode=yes; shift ;;
            -h|--help) usage ; shift ;;
            --) shift ; break ;;
             *) echo "Internal error!" ; exit 1 ;;
          esac
        done
    
        for arg do
          BUILD_TARGET=$arg; break;
        done

        if [ $android_mode = "yes" ]; then
          #override user and host for android
          BUILD_USER="user"
          BUILD_HOST="linux"
        fi

	[ -z $BUILD_TARGET ] && echo "build target is required!" && usage
        echo "--------------------------------------"
        echo "target    ====>  '$BUILD_TARGET'"
        echo "user      ====>  '$BUILD_USER'"
        echo "host      ====>  '$BUILD_HOST'"
        echo "version   ====>  '$BUILD_VERSION'"
        echo "--------------------------------------"
}

update_release_version(){
        local version=${BUILD_VERSION}`scripts/setlocalversion`
        echo "release version=\"$version\""
        sed -i "s/CONFIG_SIGMA_RELEASE_VERSION.*/CONFIG_SIGMA_RELEASE_VERSION=\"${version}\"/g" .config
}

BUILD_TARGET=
BUILD_USER=
BUILD_HOST=
BUILD_VERSION=


#parse params
parseparam "$@"
#env declare
export KBUILD_BUILD_USER=$BUILD_USER
export KBUILD_BUILD_HOST=$BUILD_HOST
#update release version for sx6
grep "CONFIG_SIGMA_DTV=y" .config -q && update_release_version
#make targets
make $BUILD_TARGET -j4

