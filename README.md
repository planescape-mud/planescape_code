# Planescape MUD build instructions

Instructions from 2015, tested on Ubuntu 14.04.

### Install packages

```bash
apt-get install -y git
apt-get install -y gcc make
apt-get install -y automake libtool
apt-get install -y bison flex
apt-get install -y gcc-multilib g++-multilib
apt-get install -y libdb5.1:i386 libdb5.1++:i386 libdb++-dev:i386
apt-get install zlib1g-dev:i386 
```

Add back packages removed by zlib1g installation:
```bash
apt-get install g++ g++-multilib gcc gcc-multilib
```

### Create folder tree

Let's assume we're under /home/psmud:

```bash
mkdir git
cd git
git clone https://yourname@bitbucket.org/psmud/planescape_world.git
git clone https://github.com/planescape-mud/planescape_code.git
mkdir runtime
mkdir objs
mv planescape_world runtime/share
```

### Compile source code

```bash
cd planescape_code
automake --add-missing
libtoolize -f
make -f Makefile.git
cd ../objs
../planescape_code/configure --prefix=/home/psmud/git/runtime
make -j 4
make install 
```

### Start MUD

```bash
cd ../runtime
./bin/planescape etc/planescape.xml &
```

### View log files

```bash
less var/log/syslog*
```


