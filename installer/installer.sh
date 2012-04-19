#!/bin/bash
echo "正在安装brasd..."
rm installer.sh

CMD="cp -r * /"

PASS=`zenity --entry --title="brasd安装脚本" --text="请输入当前用户的密码" --entry-text "" --hide-text`
if [[ $PASS = "" ]]; then
    echo "安装未完成."
    exit
fi

echo $PASS | sudo -S $CMD

echo "生成启动语句"
RC_LOCAL="/etc/rc.d/rc.local"
if [ ! -e $RC_LOCAL ]; then
    RC_LOCAL="/etc/rc.local"
    if [ ! -e $RC_LOCAL ]; then
        RC_LOCAL="/etc/init.d/rc.local"
        if [ ! -e $RC_LOCAL ]; then
            zenity --error --title "brasd安装脚本" --text "无法找到rc.local，安装失败"
            exit
        fi
    fi
fi

if [[ ! `grep brasd $RC_LOCAL` ]]; then 
    echo $PASS | sudo -S bash -c "echo '/usr/local/sbin/brasd' >> $RC_LOCAL"
fi

echo "启动brasd"
echo $PASS | sudo -S mkdir /var/run/xl2tpd/
echo $PASS | sudo -S touch /var/run/xl2tpd/l2tp-control
echo $PASS | sudo -S brasd

zenity --info --title "brasd安装脚本" --text "安装成功"
