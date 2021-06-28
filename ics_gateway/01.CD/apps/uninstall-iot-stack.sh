#!/bin/bash
# Uninstalled
systemctl stop it5-manager
systemctl disable it5-manager
redis-cli flushall
rm -rf /usr/bin/it5-loader
rm -rf /root/it5-main
rm -rf /root/it5-cli
rm -rf /data/it5/*
rm -rf /var/log/it5/*

rm -rf /etc/systemd/system/it5-manager.service

#aarch64-linux-gnu-gcc