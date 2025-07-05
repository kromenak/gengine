# Gabriel Knight 3: Blood of the Sacred, Blood of the Damned
This is a full rewrite of the Gabriel Knight 3 engine. It uses the same assets and data files as the original game, but the underlying engine is completely rewritten to be cross-platform and support modern systems.

## Data Files
When you first run the game, you will get an error message about missing data files. In order to play the game, you will need the data files - purchase a copy of GK3 (available on Steam or GOG) to get them. The data files are all the files in the `Data` directory after installing the game (`brn` asset bundles and `bik`/`avi` video files).

On Windows and Linux, copy the data files from their original location to the `Data` folder in the same directory as this README. On Mac, copy them to `Gabriel Knight 3.app/Contents/Resources/Data`.

The [Gabriel Knight 3 demo](https://archive.org/details/GabrielKnight3BloodOfTheSacredBloodOfTheDamnedDemo) is also supported. Simply copy `Gk3demo.brn` into the `Data` folder.

## Config Options
The file `GK3.ini` in the game's root directory provides some options for changing how the game functions. If interested, take a look - all options are present and documented. Just uncomment an option to have it read by the game.

## Developer Mode
You can open the developer console using the `~` key, which allows you to enter commands in the Sheep language. For example, try `SetLocation("LBY")` or `PrintString("Hello World")`. Check out the GitHub wiki for a list of all Sheep functions that can be called.

Use the `Tab` key to toggle developer tools mode - this gives you access to a menu for enabling debug visualizations or viewing scene/asset info. More tools and features will be added to this mode over time.

## Troubleshooting
This section contains notes or tips to help troubleshoot common issues when running the game.

### Mac Gatekeeper Issues
The Mac app is not code signed (purely to avoid the $99 yearly fee), so your Mac will warn you that GK3 is potentially malicious software and refuse to open it. You will need to jump through a few hoops to open the app. See [this Apple support page](https://support.apple.com/en-us/102445) for guidance.

If you're uncomfortable running unsigned programs on your Mac, you can alternatively download the source code, review it, and build/run it yourself!

### Linux Performance Issues
If the game exhibits poor performance on your Linux system, ensure that you've installed the latest drivers for your graphics card. The default driver provided out-of-the-box by Linux is likely not very effective for 3D rendering. In my testing, the default driver was only able to achieve 15-20 FPS. After installing the latest NVIDIA driver, I was able to achieve 60 FPS consistently.

## Credits
The project relies heavily on the past work of the team at Sierra who devised and built GK3 back in the mid 90s. They deserve the majority of the credit here.

This engine rewrite was primarily done by:
	Clark Kromenaker
	kromenak@gmail.com
	clarkkromenaker.com

Visit this project's GitHub page to report issues or to contribute: https://github.com/kromenak/gengine
