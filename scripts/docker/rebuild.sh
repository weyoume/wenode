./scripts/prerebuild.sh ;
git pull origin dev \
&& \
docker build -t lopudesigns/wenode-test . ;