#!/bin/sh
PRO_NAME=/home/wangkuan/work/code/git_root/gbsppt/server/10-app/nvr/gbsppt
sudo ${PRO_NAME} &
while true ; do
    NUM=`ps aux | grep ${PRO_NAME} | grep -v grep |wc -l`
    #    少于1，重启进程
    if [ "${NUM}" -lt "1" ];then
        sleep 60
        ${PRO_NAME}
        #    大于1，杀掉所有进程，重启
    elif [ "${NUM}" -gt "1" ];then
        killall -9 $PRO_NAME
        sleep 60
        ${PRO_NAME}
    fi
    #    kill僵尸进程
    NUM_STAT=`ps aux | grep ${PRO_NAME} | grep Z | grep -v grep | wc -l`

    if [ "${NUM_STAT}" -gt "0" ];then
        killall -9 ${PRO_NAME}
        sleep 60
        ${PRO_NAME}
    fi

    sleep 5
done
exit 0

