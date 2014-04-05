git submodule init
git submodule update --init --recursive
git submodule foreach --recursive git checkout master
git submodule foreach --recursive git pull
