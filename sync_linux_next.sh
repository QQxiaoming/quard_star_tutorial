#!/bin/bash
set -e

cd linux-next
CURRENT_HASHID=$(git rev-parse dev/quard_sync)
git fetch upstream
git checkout master
git reset --hard upstream/master
git push origin --force
git checkout master -b test
git cherry-pick $CURRENT_HASHID
git checkout dev/quard_sync
git reset --hard test
git branch -d test
git push origin --force
