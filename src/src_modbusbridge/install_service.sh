#!/bin/bash


sudo cp modbusbridge.service /etc/systemd/system/modbusbridge.service
sudo systemctl daemon-reload
sudo systemctl start modbusbridge.service
sudo systemctl enable modbusbridge.service
sudo systemctl status modbusbridge.service
