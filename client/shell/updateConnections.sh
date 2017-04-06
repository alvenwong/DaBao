#!/bin/sh

sed -i "6s/.*/connections = $1/" ../settings.cfg
