#!/bin/bash
set -e
export IMAGE_NAME="WeYouMe/wenode:$BRANCH_NAME"
if [[ $IMAGE_NAME == "WeYouMe/wenode:stable" ]] ; then
  IMAGE_NAME="WeYouMe/wenode:latest"
fi
sudo docker build -t=$IMAGE_NAME .
sudo docker login --username=$DOCKER_USER --password=$DOCKER_PASS
sudo docker push $IMAGE_NAME
sudo docker run -v /var/jenkins_home:/var/jenkins $IMAGE_NAME cp -r /var/cobertura /var/jenkins
cp -r /var/jenkins_home/cobertura .
