## 1. ESP RainMaker

### 1.1 Environment Setup

Additional setup that needs to be done for integrating [ESP RainMaker](https://rainmaker.espressif.com/):

*   Get the repository:
    ```
    $ git clone https://github.com/espressif/esp-rainmaker.git
    ```
*   Setting cloud_agent:
    ```
    $ export CLOUD_AGENT_PATH=/path/to/esp-rainmaker
    ```
*   Menuconfig changes:
    ```
    $ idf.py menuconfig
    -> Voice Assistant Configuration -> Enable cloud support -> enable this
    -> ESP RainMaker Config -> Use Assisted Claiming -> enable this
    ```

### 1.2 Device Provisioning

Instead of using the ESP Alexa app, Use the RainMaker apps with Alexa integration:

**iOS**: Coming soon <br>
**Android**: [Android app]()

*   Launch the app and sign-in.
*   Click on add device and scan the QR code complete the Wi-Fi setup.
*   After that you will be asked to Sign in with Amazon.
*   The phone app will verify and complete the setup after that.

### 1.3 Customisation

To customise your own device, you can edit the file components/app_cloud/app_cloud_rainmaker.c. You can check the examples in ESP RainMaker for some more device examples.

## 2. Smart Home

(This is just for amazon_aia and amazon_alexa examples.) One way to add the smart home functionality is to use [ESP RainMaker](#1-esp-rainmaker), and the other way is to use *components/app_smart_home*. For amazon_aia, this is initialized by default in the appilication.

### 2.1 Usage

Once provisioning is done and the device has booted up, the smart home feature of the device can be used via voice commands or through the Alexa app.

Example: By default, the device configured is a 'Light' with 'Power' and 'Brightness' functionalities. Voice commands like 'Turn on the Light' or 'Change Light Brightness to 50' can be used. In the Alexa app, this device will show up as 'Light' and the Power and Brightness can be controlled.

### 2.2 Customisation

To customise your own device, you can edit the file components/app_smart_home/app_smart_home.c. You can refer the files *components/voice_assistant/include/smart_home.h* and *components/voice_assistant/include/alexa_smart_home.h* for additional APIs.

A device can have the following types of capabilities/features/parameters:
*   Power: A device can only have a single power param.
*   Toggle: This can be used for params which can be toggled. Example: Turning on/off the swinging of the blades in an air conditioner.
*   Range: This can be used for params which can have a range of values. Example: Changing the brightness of a light.
*   Mode: This can be used for params which need to be selected from a pre-defined set of strings. Example: Selecting the modes of a washing machine.

## 3. Audio Player

The audio player (*components/voice_assistant/include/audio_player.h*) can be used to play custom audio files from any source (http url, local spiffs, etc.).

The focus management (what is currently being played) is already implemented internally by the SDK.

For amazon_aia: Speech/Alert/Music from Alexa has higher priority than what is played via the audio_player. So for example, if custom music is being played via the audio_player, and a query is asked, then the music will be paused and the response from Alexa will be played. Once the response is over, the music will be resumed (unless already stopped). Basically, all Alexa audio gets priority over custom audio.

For amazon_alexa: Speech/Alert from Alexa has higher priority than what is played via the audio_player. So for example, if custom music is being played via the audio_player, and a query is asked, then the music will be paused and the response from Alexa will be played. Once the response is over, the music will be resumed (unless already stopped). Another example, if custom music is being played via audio_player, and a query is asked for playing music via the cloud, then the custom music will be stopped and the music from Alexa will take over. If Alexa music was playing and custom music is played, then Alexa music will stop and the custom music will take over. Basically, music has the same priority from whichever source it is being played from. All other Alexa audio gets priority over music.

### 3.1 Enabling Custom Player

The *components/custom player* is an example using the audio_player. The default example of the custom player can play from http url and/or local spiffs and/or local sdcard but can be easily extended to play from any other source.

Easiest way to try custom player is using http url.

*   Include *custom_player.h* in the application and call *custom_player_init()* after the voice assistant early initialisation has been done.

When the application is now built and flashed on the device, the custom player will play the 3 files showing the usage of the audio_player.

### 3.2 Customisation

The default custom player just has a demo code which can be used as a reference to build your own player. The audio player, for now, just supports mp3 and aac audio formats for http urls and only mp3 audio format for local files.

## 4. Equalizer
(This is just for amazon_alexa example.) Equalizer lets you control the Bass, Mid-Range and Treble of the audio. You can use the following commands to get the values for the equalizer:
*   Set Treble to 3
*   Set Bass to -3
*   Reset Equalizer
*   Set Movie mode        // check this

The SDK will give a callback to the application (in equalizer.c) with the respective values for the equalizer. The application can then use these values and adjust the audio output.

### 4.1 Enabling Equalizer

To enable the equalizer along with Alexa:
*   Include *alexa_equalizer.h* in the application and call *alexa_equalizer_init()* before the voice assistant initialisation has been done.
