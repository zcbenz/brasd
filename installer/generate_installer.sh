#!/bin/bash
if [ `basename $PWD` = "installer" ]; then
    cd ..
fi

#安装编译好的程序到临时目录
TMPDIR=/tmp/brasd_installer
DESTDIR=$TMPDIR make install > /dev/null
cp ./installer/installer.sh $TMPDIR
mkdir $TMPDIR/usr/sbin
cp /usr/sbin/xl2tpd $TMPDIR/usr/sbin/
./installer/makeself.sh $TMPDIR "brasd_installer.sh" "Bras Client Installer" ./installer.sh
rm -rf $TMPDIR
