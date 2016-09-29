#!/bin/bash
find -L $1 -mtime +7 -type l
