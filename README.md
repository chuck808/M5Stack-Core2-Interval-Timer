# M5Stack-Core2-Interval-Timer
Countdown Interval Timer using M5Stack Core2 &amp; LovyanGFX - aka. HIIT, Gym, Pomodoro, CrossFit Timer


<!-- ABOUT THE PROJECT -->
## About The Project

This project was based on the [Wio Terminal Timer](https://www.hackster.io/SeeedStudio/wio-terminal-timer-6afe8c/) and was developed to see if I could get the LovyaGFX library working with the M5Core2 library. It also demonstrates how use a sound file with the Core2 without an external library.

![Test Image](images/1_set_num_reps.png)

### Built With

* [M5Stack Core2](https://shop.m5stack.com/products/m5stack-core2-esp32-iot-development-kit)  
* [ARDUINO IDE 2](https://www.arduino.cc/en/software) 
* [LovyanGFX](https://github.com/lovyan03/) 
* [Lang-Ship Tools Image Conversion](https://lang-ship.com/tools/image2data/)
* [Lang-Ship Tools Audio Conversion](https://lang-ship.com/tools/wav2data/)


<!-- GETTING STARTED -->
## Getting Started

Ensure you have the current software and libraries installed, upload the code and it should work streight out of the box.
If you decide to use your own welcome image and audio, prepare them in the usual way using the Lang-Ship links. There are other scripts that can achieve this.


<!-- USAGE EXAMPLES -->
## Usage

This countdown interval timer can be used in all kinds of activities right out the box, such as Boxing, HITT training, CrossFit, Sprint traning and so on.
It could also be easily adapted to suit other situations (you could even boil an egg with it :) )

Currently, the flow of the app is fairly straightforward:-

- Welcome Screen (Splash Screen)
- Setup Screens
	- Set Number of Reps
	- Set Workout Timer
	- Set Rest Timer
- Run Timer Screens
	- Run Workout Timer
	- Run Rest Timer
	- Repeat for Set Number of Reps
- End Timer Screen


<!-- FUTURE UPDATES -->
## Funture Improvements

* The audio beeps could be improved 
* The Set Num Reps should not go past sero
* The M5.BtnC.pressedFor(500) needs inproving
* Reduce flicker, even though using Sprites
* Allow the Set Modes to navigate beckwards
* RESTART current set times instead of just RESET


