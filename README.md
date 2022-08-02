# precision-touchpad-advanced-gestures
A simple-to-use Windows Precision Touchpad API and also an advanced gesture configurator for features such as 5 finger gestures

First off, all the Windows HID API handling code is from [ichisadashioko/windows-touchpad](https://github.com/ichisadashioko/windows-touchpad), as this is way beyond my skill level.

Although I did simplify it quite a bit, also support some touchpads (my touchpad) that reports each touch point as a separate... inputs? Check this [issue](https://github.com/emoacht/RawInput.Touchpad/issues/1) raised by the awesome [kamektx/TouchpadGestures_Advanced](https://github.com/kamektx/TouchpadGestures_Advanced) dev.

![image](https://user-images.githubusercontent.com/39593345/182168480-5d4ab3ac-8206-4172-a1a8-79685f3c94cc.png)
I'm thinking of not making a GUI and just take a configuration file as input. There is no gesture recognition yet, but 5 point absolute position touch input is working great now.

![image](https://user-images.githubusercontent.com/39593345/182420203-9c5a3a56-a10c-4748-90bb-8642be30facd.png)
Maybe a little on-screen display when you are performing a gesture? Idk I'm just testing out windows.
