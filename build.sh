#!/bin/bash
cd `dirname $0`

cd skynet
make linux
cd ..

cd skynet-ext
make linux
cd ..

cp skynet-ext/luaclib/* bin/luaclib
cp skynet-ext/cservice/*.so bin/cservice
cp skynet/luaclib/*.so bin/luaclib
cp skynet/cservice/*.so bin/cservice
cp -r skynet/lualib/* bin/lualib
cp -r skynet/service/* bin/service
cp skynet/skynet bin/