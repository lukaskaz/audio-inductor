#!/bin/bash

AUDIOFIFO=/tmp/cavaout
PIN=15 
CURR=0 
PREV=$CURR

function _gpio()
{
	local MODE=$1
	local VAL=$2
	gpio $MODE $PIN $VAL
}

function _gpioout()
{
	_gpio mode out
}

function _gpioset()
{
	_gpio write 1
}

function _gpioreset()
{
	_gpio write 0
}

function _gpiotoggle()
{
	_gpio toggle
}

function _gpioread()
{
	return $(_gpio read)
}

function _cleanup()
{
	_gpioreset
}

trap _cleanup EXIT

##### main #####
_gpioout
while read CURR; do
	if [ $CURR -ne $PREV ]; then 
		echo $CURR
		PREV=$CURR
		_gpiotoggle
	fi
done < <(cat $AUDIOFIFO)

##### END #####

