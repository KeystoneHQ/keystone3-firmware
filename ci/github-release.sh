#!/bin/bash

set -e
cd $(dirname $0)/..
source ./ci/prepare-release-info.sh

# Github Repo
REPO_OWNER="KeystoneHQ"
REPO_NAME="keystone3-firmware"

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
