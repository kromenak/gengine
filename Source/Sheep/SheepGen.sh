#!/bin/sh

# Executes flex and bison to regenerate Sheep Scanner/Parser files.
flex Sheep.l
bison -d Sheep.yy
