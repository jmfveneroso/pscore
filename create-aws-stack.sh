#!/bin/bash

unset PYTHONPATH

TemplateBucket=jmfveneroso

array=(
deployment-pipeline.yaml
ecs-cluster.yaml
load-balancer.yaml
service.yaml
vpc.yaml
)

for f in "${array[@]}"; do
  aws s3api put-object --bucket jmfveneroso --key templates/$f --body cloudformation/$f
done

aws cloudformation create-stack --stack-name website --template-body file://$1 --capabilities CAPABILITY_IAM --parameters \
  ParameterKey=GitHubUser,ParameterValue=jmfveneroso \
  ParameterKey=GitHubToken,ParameterValue=5d0b07b4021c23ebe90584e825b80e73d65b2bc8
