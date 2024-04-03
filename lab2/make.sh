#!/bin/bash 
sudo make
sudo rmmod lab2
sudo insmod lab2.ko
sudo fdisk -l /dev/our_disk
