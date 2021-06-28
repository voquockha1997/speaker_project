# 1. Setup Cross compile aarch64-linux-gnu-gcc
sudo apt install gcc-aarch64-linux-gnu
sudo apt install g++-aarch64-linux-gnu

- check path
path = which aarch64-linux-gnu-gcc
cd path
sudo ln -s aarch64-linux-gnu-gcc arm-linux-gcc
sudo ln -s aarch64-linux-gnu-g++ arm-linux-g++
# 2. Run script to build
    build.sh <target> <product_name> <major> <minor> <rev>"
         target             x86|raspi"
         product_name       the name of product"
    Examples:"
         ./build.sh x86 DEMO 01 01 01"
         ./build.sh raspi BTS_GATEWAY 01 01 01"

# 3. Setup GPIO One Wire
-First step:
 cd /boot/firmware/
   nano config.txt
   add "dtoverlay=w1-gpio,gpiopin=18"
   sudo reboot
 
-Second step:
 sudo modprobe w1-gpio && sudo modprobe w1_therm

# 4. Init GPIO
- GPIO 18 for DS18B20
- GPIO 23 for Relay 1 (Trigger Air Conditional 1)
- GPIO 24 for Relay 2 (Trigger Air Conditional 2)

# 5.OS:
- using os: ubuntu-18.04.5-preinstalled-server-arm64+raspi3.img.xz

# 6.SSH
- Access to device with user: it5, password: Vnpt@123