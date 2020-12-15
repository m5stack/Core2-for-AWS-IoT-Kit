## ChangeLog

### v1.0b3 2020-10-28

**Enhancements**
* Added support for smart home.
* Added support for audio_player and custom_player.
* Added some basic prompts. Also added support to add more custom prompts.
* Using security1 instead of security0 in local control.

**API Changes**
* Added examples/additional_components/ for cloud integration and smart-home integration.
* Moved app_cloud from examples/common/ to examples/additional_components/
* Updated the ESP RainMaker cloud integration to use the new APIs.
* Added <voice_assistant>_early_init() which can initialise the audio pipeline before provisioning to support playing of prompts and other audio.
* Changed the adding of endpoints in provisioning to use the event_handlers. This now happens internally.

**Bug Fixes**
* Fixed various stability issues related to the connection with the server.
* Fixed crash when device certificates are missing.
* Fixed memory leak which in turn cause the device to be stuck.
* Fixed wifi-reset happening instead of factory-reset in some cases.

### v1.0b2 - 2020-07-17

**Enhancements**
* Various restructurings to support custom audio_boards.
* Support for integrating ESP RainMaker.
* Adding hollow_dsp for using custom dsp driver.
* Adding led_patterns for customisation.
* Adding va_ui to support UIs other than LEDs.

**API Changes**
* Added examples/common/ directory.
* The audio_board files path is changed from board_support_pkgs to components/audio_hal

**Bug Fixes**
* Fixed continuous disconnect from the server.
* Handled multiple acknowledgements from server for connect request which in-turn caused disconnection.
* Fixed various bugs which caused audio to stop during barge-in.

### v1.0b1 (Beta) - 2020-03-06

**Enhancements**

* Basic conversation
* Multi-turn conversations
* Audio Streaming and Playback: Amazon music
* Audio Book Support: Kindle, Audible
* Volume control via voice command
* Alarms, Timers, Reminders, Notifications
* Companion Phone app for configuration and control: Android, iOS
* AWS IoT (MQTT and Shadow) support

**Known Issues**

* You have to reboot the device after setup for the wake-word to work.
