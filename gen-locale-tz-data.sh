#!/bin/bash
# You should run this program on a UNIX system that uses locale(5) files.
python3 gen-tz-data.py > include/x_datetime_timezone.h
python3 gen-locale-data.py > include/x_datetime_locale_data.h

