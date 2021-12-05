
# Playr Remote

A BLE Keyboard on ESP32 (that can be be used on Home Assistant) to control devices over bluetooth.

This project is inspired on [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) but it has modifications to be recognized as an Alexa Gadget.

The main purpose of this project is able to control my Alexa devices directly from Home Assistant, without any API, as a remote control. The objective was control media: Next, Prev, Volume Up and Down, Play and Pause, and Mute. It's working fine with Echo 4! (I haven't tested it on other Alexa devices yet)

Futhermore, you can control your Android TV, Windows PC, or other devices that accept a BLE keyboard. For example, tested on Android Mi TV Stick: You can turn on, turn off, control the volume, type text, etc...

# How it works?

Once running, the ESP32 will emulate a bluetooth keyboard. The ESP32 LED will blink while there is no any device connected. The devices must search and pair with "Playr Remote". 

Once connected, you will be able to send commands through the serial port to ESP32 which will map the keyCodes and send them to the bluetooth client.

This is a example of command that can be sent:

	P;116\n

Explanation: 'P' means that you want just press and release a key, the ';' is a separator, the '116' is a KEY_POWER of Linux Input Codes, you can see all on this list: [Input event codes from Linux](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h)
The '\n' is a line break separator, is mandatory to end a command.
When you send this command to ESP over serial port, the esp will send the KEY_POWER to connected device.

You can create an automation on Home Assistant to send the commands to ESP. 
  
# Commands

The structure of the commands is simple. The command char (P or D or U), the separator ';' the keycode and the linebreak '\n' to end the command.

	P;(keyCode)\n
	D;(keyCode)\n
	U;(keyCode)\n
	
- P: is used when you wish send a keyUp and keyDown command, this PRESS and RELEASE the key immediately
- D: is a keyDown command, this PRESS the key on the keyboard
- U: is a keyUp command, this RELEASE the key on the keyboard

Example  - Increasing volume: 
When you sent the keydown volume "**D;115\n**", the volume in your device will increase until you send "**U;115\n**" command.  After pressing a key with the **D** command, never forget to release it with the **U** command

Example: Typing the character "A" (capitalized). On your real keyboard, probably you need to press **SHIFT** and **a**, to make this, you must send 4 events: Press KEY_LEFTSHIFT (42),  Press KEY_A(30), Release KEY_A(30), Release KEY_LEFTSHIFT (42).

	D;42\n
	D;30\n
	U;30\n
	U;42\n
    
Example: Control + ALT + DELETE. Press KEY_LEFTCTRL (29), Press KEY_LEFTALT(56), Press KEY_DELETE(111), Release KEY_DELETE(111), Release KEY_LEFTALT(56), Release KEY_LEFTCTRL(29) 

	D;29\n
	D;56\n
	D;111\n
	U:111\n
	U:56\n
	U:29\n


