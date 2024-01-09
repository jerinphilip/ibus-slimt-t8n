#!/bin/bash

set -eo pipefail
set -x

REMOTE="git@github.com:jerinphilip/ibus-slimt-t8n.git"
FOSSIL=$(realpath ibus-slimt-t8n.fossil)
REPO=$(mktemp -d ibus-slimt-t8n-XXX -p /tmp)
mkdir -p $REPO
git -C $REPO init
git -C $REPO checkout -b trunk
git -C $REPO remote add origin $REMOTE

fossil export --git --export-marks fossil.marks \
  $FOSSIL | git -C $REPO fast-import \
  --export-marks=git.marks

# fossil export --git $FOSSIL | git -C $REPO fast-import
git -C $REPO log
git -C $REPO branch -m main
git -C $REPO push origin main --force
rm -rf $REPO
