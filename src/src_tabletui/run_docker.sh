#!/bin/bash
docker run -itd -p 3000:3000   --network host --name tabletui --restart unless-stopped  tabletui:latest
