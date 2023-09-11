#!/usr/bin/env bash

set -e
cd $(dirname $0)/..
source ci/env

image=623147552995.dkr.ecr.eu-central-1.amazonaws.com/keystone3_project_pillar_firmware:latest

docker run -v $(pwd):/project-pillar-firmware $image python3 build.py -e ${ENVIRONMENT} -p ${PURPOSE} -o ${OPTIONS}

if [ "$PURPOSE" == "debug" ]; then
	echo -e "\033[1;31m firmware without signature"
else
	ota_maker_image=623147552995.dkr.ecr.eu-central-1.amazonaws.com/keystone3_ota_maker:latest

  echo -e "\033[1;31m sign the firmware"

  docker run -v $(pwd)/build:/build \
  -e AWS_DEFAULT_REGION=$AWS_DEFAULT_REGION \
  -e AWS_ACCESS_KEY_ID=$AWS_ACCESS_KEY_ID \
  -e AWS_SECRET_ACCESS_KEY=$AWS_SECRET_ACCESS_KEY \
  -e AWS_SESSION_TOKEN=$AWS_SESSION_TOKEN \
  $ota_maker_image --source /build/mh1903.bin --destination /build/keystone3.bin
  echo "uploading artifacts"
  export BUILDKITE_S3_ACCESS_KEY_ID=$AWS_ACCESS_KEY_ID
  export BUILDKITE_S3_SECRET_ACCESS_KEY=$AWS_SECRET_ACCESS_KEY
  export BUILDKITE_S3_SESSION_TOKEN=$AWS_SESSION_TOKEN
  export BUILDKITE_ARTIFACT_UPLOAD_DESTINATION="s3://keystone-g3-firmware/$BUILDKITE_BRANCH/$BUILDKITE_BUILD_NUMBER/"
  export BUILDKITE_S3_ACCESS_URL="https://keystone-g3-firmware.s3.eu-central-1.amazonaws.com"
  export BUILDKITE_S3_DEFAULT_REGION="eu-central-1"
  export BUILDKITE_S3_ACL="bucket-owner-full-control"
  cd build
  buildkite-agent artifact upload "mh1903.*"
  buildkite-agent artifact upload "keystone3.bin"
fi