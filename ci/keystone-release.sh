#!/bin/bash

set -e
cd $(dirname $0)/..
source ./ci/prepare-release-info.sh


S3_BUCKET_NAME="keystone-contents"
ASSET_OBJECT_KEY="contents/KeystoneFirmwareG3/v$VERSION/$ASSET_FILE_NAME"

if [ "$ENV" = "production" ]; then
  KEYSTONE_RELEASE_DOMAIN="https://api-internal.keyst.one"
else
  KEYSTONE_RELEASE_DOMAIN="https://api-staging-internal.keyst.one"
fi


# Upload Asset to S3
echo "Upload asset to S3"
S3_ASSET_PATH="s3://$S3_BUCKET_NAME/$ASSET_OBJECT_KEY"

if aws s3 ls "$S3_ASSET_PATH" 2>&1 | grep -q "$ASSET_FILE_NAME"; then
  echo "Asset already exist in S3, verifying"
  aws s3 cp "$S3_ASSET_PATH" "./tmp/s3/$ASSET_FILE_NAME"
  s3_asset_checksum=$(sha256sum "./tmp/s3/$ASSET_FILE_NAME" | cut -d " " -f 1)
  if [ "$checksum" != "$s3_asset_checksum" ]; then
    echo "Checksums do not match. Exiting."
    exit 1
  else
    echo "Asset in S3 is identical to build result, skip upload."
  fi
else
  echo "Upload asset to S3"
  aws s3 cp "./tmp/$ASSET_FILE_NAME" "$S3_ASSET_PATH"
fi


# Create Release in Keystone
echo "Start create Keystone release $VERSION for $ENV"
KEYSTONE_RELEASE_URL="$KEYSTONE_RELEASE_DOMAIN/v1/web/firmware_release/"
release_response=$(curl -L \
  -X POST "$KEYSTONE_RELEASE_URL" \
  -F 'version="'"$VERSION"'"' \
  -F 'changelog_en="'"$change_log_en"'"' \
  -F 'changelog_zh="'"$change_log_zh"'"' \
  -F 'download_url="'"$ASSET_OBJECT_KEY"'"'
  )

release_success=$(echo "$release_response" | grep -o -m 1 '"success": [a-z]*' | awk -F': ' '{print $2}')

if [[ "$release_success" == "true" ]]; then
  echo "Keystone release successfully."
  echo "$release_response"
else
  echo "Failed to create Keystone release."
  echo "$release_response"
  exit 1
fi
