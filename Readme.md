# Sauna control #

This is a control board for the sauna equipment. It includes thermostat and recycling motor control.


## Get sources ##

```
git clone https://github.com/specadmin/sauna-control
cd sauna-control
git submodule update --init
```


## Compile  ##

`make`

Make sure you have `avr-gcc` and `avr-libc` installed before running this command.


## Upload ##

`make install`

This command uses `avrdude` for uploading the firmware. See Makefile for details.



## Code modification ##

Use Code::Blocks or any other IDE for code modification.


