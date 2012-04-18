#!/bin/bash
if [ `basename $PWD` = "installer" ]; then
    cd ..
fi

#安装编译好的程序到临时目录
TMPDIR=/tmp/brasd_installer
DESDIR=$TMPDIR make install > /dev/null
./installer/makeself.sh $TMPDIR "brasd_installer.sh" "Bras Client Installer"
rm -rf $TMPDIR
