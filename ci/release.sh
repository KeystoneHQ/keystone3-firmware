#!/bin/bash

set -e

cd $(dirname $0)/..

source ci/env

repository=623147552995.dkr.ecr.eu-central-1.amazonaws.com/$app_name

image=$repository:$image_tag

docker build -t $image .

aws ecr get-login-password --region eu-central-1 | docker login --username AWS --password-stdin 623147552995.dkr.ecr.eu-central-1.amazonaws.com/$app_name

docker push $image

docker tag $image $repository:latest

docker push $repository:latest

