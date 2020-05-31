
### Build Dependency
sudo apt install libprotobuf-c-dev autoconf libreadline-dev

### Update Git 3rd Submodules && Patch Skynet Witch Excel C Services
``` shell
$ git submodule update --init
$ cd skynet
$ git apply ../patches/skynet_load_excel.patch
$ cd ..
```
### Build Project
``` shell
$ ./build.sh
```


### Start Server
``` shell
$ bin/start.sh
or
$ cd bin/
$ ./start
```

