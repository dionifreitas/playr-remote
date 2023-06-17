
# Playr Remote

A BLE Keyboard on ESP32 (that can be be used on Home Assistant) to control devices over bluetooth.

This project is inspired on [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard).

The main purpose of this project is able to control my devices directly from Home Assistant, without any API, as a remote control. The objective was control media: Next, Prev, Volume Up and Down, Play and Pause, and Mute.

Futhermore, you can control your Android TV, Windows PC, or other devices that accept a BLE keyboard. For example, tested on Android Mi TV Stick: You can turn on, turn off, control the volume, type text, etc...

Warning: This was developed and tested on DOIT ESP32 ESP-WROOM-32.

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

# Home Assistant 

The connection between ESP32 and Home Assistant will be done through the serial port. This is enough to send commands to your devices: turn on, turn off, play... etc. But if your intention is create a remote controller for Alexa (it is my case), you can follow the steps below:

Prerequisites:

The [Node-RED](https://community.home-assistant.io/t/home-assistant-community-add-on-node-red/55023) running on HA to create automations more easily.

The [keyboard_remote](https://www.home-assistant.io/integrations/keyboard_remote/) integration to receive signals from a keyboard and use it as a remote control.

You can buy a generic remote control like the below to be used in Home Assistant; if not, you can use a normal keyboard (you keyboard must have media keys if you wish control Alexa):

![remote](https://user-images.githubusercontent.com/8365145/144768620-b4a1cbbc-e6cd-4bb6-87ff-ca0b7be76b94.jpg)

This is known as an Air Mouse, basically it's just a keyboard and mouse for Android TV boxes. You can also use it on your computer. The device is recognized as a simple keyboard. They come with a dongle, but there is a bluetooth version. You need to connect the keyboard to Home Assistant using the dongle or over bluetooth. I haven't tested it with the bluetooth version. 

My Home Assistant is running on Raspiberry Pi 4 Model B. 
The ESP32 is connected directly to Raspiberry over USB.

How to start: 
Use Arduino IDE to compile on your ESP32 and then, connect over usb to Home Assistant. Connect the keyboard too.

Once connected, you must identify your keyboard. (On HA, go to Supervisor > System > Host > Hardware), you will se something like: 

![image](https://user-images.githubusercontent.com/8365145/146999507-2789311a-5472-4a51-b3b1-733ffd7f030b.png)

My Keyboard was recognized with 4 input devices (event0, event1, event2, event3). Maybe your keyboard have only one.

Enable [keyboard_remote](https://www.home-assistant.io/integrations/keyboard_remote/) integration (it is a native integration, you just add to configuration.yaml). 
This is my configuration:
```yaml
keyboard_remote:
  - device_descriptor: "/dev/input/event0"
    emulate_key_hold: false
    type:
      - "key_down"
      - "key_up"
  - device_descriptor: "/dev/input/event1"
    emulate_key_hold: false
    type:
      - "key_down"
      - "key_up"
  - device_descriptor: "/dev/input/event2"
    emulate_key_hold: false
    type:
      - "key_down"
      - "key_up"
  - device_descriptor: "/dev/input/event3"
    emulate_key_hold: false
    type:
      - "key_down"
      - "key_up"
```
Verify configuration, save and reboot.

On Node-RED we will listen for keyboard events and forward over bluetooth to devices:

This is a very basic example: All keys events of keyboard will redirected to target device. 

![image](https://user-images.githubusercontent.com/8365145/147002326-6f4cbfd5-48f6-43e8-844f-0d7a34a4f56c.png)

Nodes configuration:

**events: all** node (aka Keyboard Received)

	Name: Keyboard Remote or other of your choice
	Event type: keyboard_remote_command_received
	Output properties:
	msg.payload = Expression: $outputData("eventData").event

![image](https://user-images.githubusercontent.com/8365145/147002930-ede3049b-1895-461e-95ad-77e7fe15948d.png)


**function** node (aka Serialize)

The **on message** code must be:

```javascript
let payload = (msg.payload.type == "key_up" ? 'U' : 'D') + ';' + msg.payload.key_code;
msg.payload = payload;
return msg;
```
![image](https://user-images.githubusercontent.com/8365145/147003274-9a57c3ab-1cc9-4737-bdb5-1367c9aa09fa.png)

**serial out** node (aka Remote)

Add a new Serial Port, on **Serial port** field specify your device, in my case was **/dev/ttyUSB0**

On Input: Split input **on the character**: **\n** and deliver **ASCII string**

On Output: **Add Character To Output Messages**: **\n** **(This is very important or commands will not work)**

![image](https://user-images.githubusercontent.com/8365145/147003958-d1c8ce96-91df-4a14-9e47-cc23076138b4.png)

You can drag a **debug node** with output: **complete msg object** to see the logs.

This is it!


