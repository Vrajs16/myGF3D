git submodule update --init --recursive
cd gfc/simple_json/src
git pull 
make clean
make 
make static

cd ../../simple_logger/src
git pull
make clean
make
make static

cd ../../src/
git pull
make clean
make
make static
