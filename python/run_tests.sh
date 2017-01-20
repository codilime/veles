#!/usr/bin/env bash
set -ex

dir=`mktemp -d`

function finish {
  rm -rf $dir
}
trap finish EXIT

tox --workdir $dir
$dir/py27/bin/testr last --subunit | subunit2junitxml --no-passthrough -o res27.xml
$dir/py35/bin/testr last --subunit | subunit2junitxml --no-passthrough -o res35.xml

