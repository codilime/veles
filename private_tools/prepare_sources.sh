#!/bin/bash

version=""
build=""
tag=""

function usage {
    echo "Usage: "
    echo "./prepare_sources.sh -v VELES_VERSION -b JENKINS_BUILD_NUMBER [ -t GIT_TAG_BRANCH_COMMIT_ID ]"
}

function get_file {
    if (( "${#}" == 3 ))
    then
        wget "http://jenkins.eax.codilime.com/job/EAX/job/eax-gui/job/master/${build}/artifact/${3}/${1}" -O "Veles_${version}_${2}.zip"
    else
        wget "http://jenkins.eax.codilime.com/job/EAX/job/eax-gui/job/master/${build}/artifact/${1}" -O "Veles_${version}_${2}.zip"
    fi
}

while getopts ":hv:b:t:" opt
do
    case $opt in
        h)
        usage
        exit 0
        ;;
        \?)
        echo "Invalid option ${OPTARG}"
        usage
        exit 1
        ;;
        :)
        echo "Option -${OPTARG} requires an argument"
        usage
        exit 1
        ;;
        v)
        version=${OPTARG}
        ;;
        b)
        build=${OPTARG}
        ;;
        t)
        tag=${OPTARG}
        ;;
    esac
done

if [ -z "${version}" ]
then
    echo "Version parameter (-v) is required"
    usage
    exit 1
fi
if [ -z "${build}" ]
then
    echo "Jenkns build id (-b) is required"
    usage
    exit 1
fi

git clone git@github.com:codilime/eax.git
cd ./eax
if [ -z "${tag}" ]
then
    git checkout ${tag}
fi

rm -rf .git private_tools
rm Jenkinsfile REVISION .gitignore CONTRIBUTING.md
cd ..

mv "eax" "veles_${version}"
tar -czvf "Veles_${version}_Source.tar.gz" "veles-${version}/"
get_file "veles-msvc2015.zip" "32bit_Windows" "build_msvc2015"
get_file "veles-msvc2015_64.zip" "64bit_Windows" "build_msvc2015_64"
get_file "veles-osx.zip" "64bit_OSX"
get_file "veles-ubuntu1604.zip" "64bit_Ubuntu1604"
