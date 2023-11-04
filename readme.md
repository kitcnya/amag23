<!-- -*- mode: markdown; coding: utf-8 -*- -->

# Customization of my Koolertron AMAG23 keyboard

![Koolertron AMAG23 w/DSA profile keycaps](amag23.png)

## How to see my keymap

You can use the site [QMK Configurator](https://config.qmk.fm/#/amag23/LAYOUT)
with my [kitcnya1.json](https://raw.githubusercontent.com/kitcnya/amag23/master/kitcnya1.json) file.

## Current status

I'm trying to customize my Koolertron AMG23 for gaming using QMK.
In order to gather basic functionality into one layer,
I am experimenting with a method of sending multiple keycodes with keystroke variations.
QMK tap dance feature seems to be usable,
but it turned out that it could not be used in the github userspace environment
because custom key codes could not be defined.

Currently, I am trying out a program that directly processes key codes
using the `process_record_user()` function for existing key codes
such as F20, F21, ... and so on that are not in use.

- F20 would be for 'J' key for primary use and 'V' key for secondary use also,
- F21 would be for 'B' key for primary use and 'M' key for secondary use also,
- F22 would be for 'L' key for primary use and 'C' key for secondary use also,
- F23 would be for 'G' key for primary use and 'U' key for secondary use also,
- F24 would be for 'Y' key for primary use and 'O' key for secondary use also.
