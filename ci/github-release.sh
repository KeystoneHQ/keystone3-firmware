#!/bin/bash

set -e
cd $(dirname $0)/..

# Github Repo
REPO_OWNER="KeystoneHQ"
REPO_NAME="keystone3-firmware"

# Release Version
CURRENT_BRANCH=$BUILDKITE_BRANCH
VERSION="${CURRENT_BRANCH#release/v}"

CHANGELOG_FILE_EN="./CHANGELOG.md"
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
change_log_en=$(read_changelog_by_version $VERSION $CHANGELOG_FILE_EN)
if [[ -z "$change_log_en" ]]; then
  echo "Failed to get changelog for $VERSION "
  exit 1
fi


# Checksum
aws s3 cp "s3://keystone-g3-firmware/$BUILDKITE_BRANCH/$BUILDKITE_BUILD_NUMBER/$ASSET_FILE_NAME" "./tmp/$ASSET_FILE_NAME"
checksum=$(sha256sum "./tmp/$ASSET_FILE_NAME" | cut -d " " -f 1)


# Create Github Release
echo "Start create github release $VERSION"

GITHUB_RELEASE_URL="https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/releases"
GITHUB_RELEASE_BODY="## Release notes Details\r\n$change_log_en\r\n## SHA-256 Checksum\r\n\`$checksum\`\r\n:exclamation::exclamation::exclamation:(please note that this is the checksum of the $ASSET_FILE_NAME file)"
BOT_RELEASE_TOKEN=$(aws secretsmanager get-secret-value --secret-id "keystoneg3/botReleaseToken" --version-stage AWSCURRENT --query SecretString --output text)

GITHUB_REQUEST_BODY='{"tag_name": "'$VERSION'","target_commitish": "'$CURRENT_BRANCH'","name": "'$VERSION'","body": "'$GITHUB_RELEASE_BODY'","draft": false,"prerelease": false}'
echo "Github Release Request Body: $GITHUB_REQUEST_BODY"

release_response=$(curl -L \
  -X POST "$GITHUB_RELEASE_URL" \
  -H "Accept: application/vnd.github+json" \
  -H "Authorization: Bearer $BOT_RELEASE_TOKEN" \
  -H "X-GitHub-Api-Version: 2022-11-28" \
  -H "Content-Type: application/json" \
  -d "$GITHUB_REQUEST_BODY"
)

if [[ "$release_response" == *"id"* ]]; then
  echo "Github release created successfully."
  echo "$release_response"
else
  echo "Failed to create Github release."
  echo "$release_response"
  exit 1
fi


## Upload Asset to Github Release
echo "Start upload asset to github release $VERSION"
release_id=$(echo "$release_response" | grep -o -m 1 '"id": [0-9]*' | awk -F': ' '{print $2}')

GITHUB_RELEASE_UPLOAD_URL="https://uploads.github.com/repos/$REPO_OWNER/$REPO_NAME/releases/$release_id/assets?name=$ASSET_FILE_NAME"
asset_upload_response=$(curl -L \
  -X POST \
  -H "Accept: application/vnd.github+json" \
  -H "Authorization: Bearer $BOT_RELEASE_TOKEN" \
  -H "X-GitHub-Api-Version: 2022-11-28" \
  -H "Content-Type: application/octet-stream" \
  $GITHUB_RELEASE_UPLOAD_URL \
  --data-binary "@./tmp/$ASSET_FILE_NAME")

asset_upload_state=$(echo "$asset_upload_response" | grep -o -m 1 '"state": [0-9]*' | awk -F': ' '{print $2}')

if [[ "$asset_upload_state" -eq "uploaded" ]]; then
  echo "Github release asset upload successfully."
  echo "$asset_upload_response"
else
  echo "Failed to upload asset to Github release."
  echo "$asset_upload_response"
  exit 1
fi
