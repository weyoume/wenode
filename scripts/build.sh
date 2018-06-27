# export origindir=$(pwd)
# rm -fr ../build-dir ;
# mkdir -p ../build-dir ;
# cp -r ./* ../build-dir/ ;
# rm -fr ../build-dir/.git ;
# mkdir -p ../build-dir/.git ;
# cp -r $(awk '{ print $2 }' .git)/* ../build-dir/.git ;
# cd ../build-dir ;
git submodule update --init --recursive ;
docker build . ;
# cd $origindir ;


