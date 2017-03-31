#!/usr/bin/env bash
set -ex

dir=`mktemp -d`

function finish {
  rm -rf $dir
}
trap finish EXIT

tox --workdir $dir
