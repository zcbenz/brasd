#!/bin/bash
echo "正在安装brasd..."
rm installer.sh

CMD="cp -r * /"

PASS=`zenity --entry --title="brasd安装脚本" --text="请输入当前用户的密码" --entry-text "" --hide-text`
if [[ $PASS = "" ]]; then
    echo "安装未完成."
    exit
fi

sudo -S $CMD

echo "完成."
