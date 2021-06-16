## EM2 TABLET UI


# FEATURES
* simple NodeJS server with SocketIO Support
* visulize different ROS topics to show all important loco information
* !its only to show loco information and has no control functions!
* provides speed, warnings, tempertures, lightstate, battery-mgmt

# SETUP
You can find a `config.json` file in this directory to setup this tablet ui.
* `"webserver_default_port":<port>,` - defines the webserver port [DEFAULT 3345]


# START
First you have to download all NodeJS packages using npm.
Run `npm install` in this directory to download them.

To start the server you have to run a `roscore`, after a `roscore` is running simply start the server with `node server` in this directory.

For easy setup you can find a startup script for the whole system.
Call `start_all.sh` in the `/src` directory.


After that you cann access the Site over the `<IP>:3345/`.
The site is optimised for the Tablet (`1280x800px`) we are using to control the loco.



# PICTURES

## THE CONZEPT LIKE FROM THE ZUSI SOFTWARE
![Gopher image](/documenation/pictures/tablet_ui_server/tablet_ui_server_konzept.JPG)


## FIRST WebUI
![Gopher image](/documenation/pictures/tablet_ui_server/tablet_ui_server_v0.JPG)
### DEMO VIDEO FROM FIRST WebUI WITH REALTIME SPEED SENSING
[![VIDEO](https://img.youtube.com/vi/dz4WAVGvBJE/0.jpg)](https://www.youtube.com/watch?v=dz4WAVGvBJE)


# TODO
* -> complete ui design
* -> test ui on tablet
* -> create docker start file
* ->add ros topics to config file
