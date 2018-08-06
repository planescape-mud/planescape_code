# Planescape MUD build instructions

Tested on Ubuntu 16.04.

### Install packages

```bash
apt-get update
apt-get install -y gcc make automake libtool bison flex gcc-multilib g++-multilib git
dpkg --add-architecture i386
apt-get update
apt-get install -y libdb5.3++:i386 libdb5.3++-dev:i386 libdb5.3:i386 zlib1g-dev:i386
```

### Create folder tree

Let's assume we're under /home/psmud:

```bash
git clone https://yourname@bitbucket.org/psmud/planescape_world.git
git clone https://github.com/planescape-mud/planescape_code.git
mkdir runtime
mkdir objs
mv planescape_world runtime/share
```

### Compile source code

```bash
cd planescape_code
make -f Makefile.git
cd ../objs
../planescape_code/configure --prefix=/home/psmud/runtime
make -j 4 && make install 
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


