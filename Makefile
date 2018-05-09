#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := Homekit

include $(IDF_PATH)/make/project.mk

buildnumber:
	./buildnumber --header "main/HAP/HAPBuildnumber.hpp"
