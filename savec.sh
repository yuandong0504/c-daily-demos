#!/bin/bash
# 用法: ./savec.sh 文件.c "提交信息"
file=$1
msg=$2
mv -f ~/$file ~/cdd/
cd ~/cdd || exit
git add "$file"
git commit -m "$msg"
git push
