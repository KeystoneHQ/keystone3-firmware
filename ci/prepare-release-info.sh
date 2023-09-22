#!/bin/bash

set -e
cd $(dirname $0)/..

# Release Version
CURRENT_BRANCH=$BUILDKITE_BRANCH
VERSION="${CURRENT_BRANCH#release/v}"

CHANGELOG_FILE_EN="./CHANGELOG.md"
CHANGELOG_FILE_CH="./CHANGELOG-ZH.md"
ASSET_FILE_NAME="keystone3.bin"

function read_changelog_by_version {
  local target_version="$1"
  local changelog_file_path="$2"

  local in_target_version=false
  local target_version_content=""

  while IFS= read -r line
  do
    # Check if it matches the title line of the target version
    if [[ "$line" == "## $target_version"* ]]; then
      in_target_version=true
    elif [[ "$line" == "## "* ]]; then
      # If encountering a new version title line, it means we have gone past the target version section, so exit the loop
      if [ "$in_target_version" = true ]; then
        break
      fi
    elif [ "$in_target_version" = true ]; then
      target_version_content="$target_version_content$line\r\n" # Add content from the target version to the variable
    fi
  done < "$changelog_file_path"

  echo "$target_version_content"
}


# Extract Changelog Content
change_log_en=$(read_changelog_by_version "$VERSION" $CHANGELOG_FILE_EN)
if [[ -z "$change_log_en" ]]; then
  echo "Failed to get changelog for $VERSION"
  exit 1
fi

change_log_zh=$(read_changelog_by_version "$VERSION" $CHANGELOG_FILE_CH)
if [[ -z "$change_log_zh" ]]; then
  echo "Failed to get changelog in Chinese for $VERSION"
  exit 1
fi



# Download Asset
aws s3 cp "s3://keystone-g3-firmware/$BUILDKITE_BRANCH/$BUILDKITE_BUILD_NUMBER/$ASSET_FILE_NAME" "./tmp/$ASSET_FILE_NAME"


# Calculate Checksum
checksum=$(sha256sum "./tmp/$ASSET_FILE_NAME" | cut -d " " -f 1)
