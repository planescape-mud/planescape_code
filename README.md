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
cd /home/psmud/runtime
./bin/planescape etc/planescape.xml &
```
Or, recommended:
```bash
./bin/autorun &
```
Autorun script will take care of restarting the server after a crash or a kill.

### View log files

```bash
less var/log/syslog*
```
### Changing a plugin
First, rebuild and reinstall the plugin you want to change. For example, you've just changed something in feniaroot plugin:
```bash
cd /home/psmud/objs/plug-ins/feniaroot
make -j 4 && make install
```
Tell the server to reload all plugins:
```bash
kill -s SIGUSR1 PID
```
Tell the server to reload just changed plugins:
```bash
kill -s SIGUSR2 PID
```
Note: when running as ./bin/autorun, you can use ``cat /home/psmud/runtime/var/run/ps.pid`` instead of PID to get the server process ID.

### Changing core code or libdreamland
First, rebuild and reinstall src or libdreamland. For example:
```bash
cd /home/psmud/objs/src
make -j 4 && make install
```
Then reboot:
```bash
kill -1 PID
```
or, from inside the game, type:
```
shutdown reboot
```
