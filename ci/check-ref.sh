#!/usr/bin/env bash
set -eu

on_protected_branch=false
branches=$(git branch -a --contains tags/$GITHUB_REF_NAME)
for branch in $branches; do
  if [[ "$branch" =~ .*origin/master.* ]]; then
    on_protected_branch=true
  fi
done

if [ "$on_protected_branch" == false ]; then
  echo "Tag is not on master branch. Abort build."
  exit 1
fi
