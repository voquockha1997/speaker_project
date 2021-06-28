#!/bin/bash
target=$1
config=$2

set -e # exit when a command fails.
set -u # exit when using undeclared variables.

CUR_PATH=`pwd`

# Prepare
echo "Prepare ..."
mkdir -p /var/log/it5/
mkdir -p /data/it5

echo "Install 3rd packages"
apt-get update
apt-get install -y libssl-dev libcrypto++-dev libcurl4-openssl-dev uuid-dev libsnmp-dev libperl-dev
apt-get install -y cpanminus make redis-server

cpanm JSON::Tiny

systemctl enable redis-server

systemctl start redis-server

# Installed
echo "Extracting iot resource..."

tar xzf $CUR_PATH/iot-resource-${target}.tar.gz -C /data/it5

chmod +x /data/it5/util/*

mv /data/it5/util/it5-main /root/

echo "Configuring Redis data..."
/data/it5/util/Config-${config}

echo "Installing it5-manager service..."
cp /data/it5/util/it5-manager.service /etc/systemd/system/
systemctl daemon-reload
systemctl enable it5-manager
systemctl stop it5-manager
systemctl start it5-manager
