# AtomCompiler
A toy compiler for fun

# Build
~~~sh
sudo apt install antlr4 libantlr4-runtime-dev
mkdir build
cd build
cmake -DUPDATE_ANTLR=ON ..
make -j `nproc`
~~~
