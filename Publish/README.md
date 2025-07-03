# Gabriel Knight 3: Blood of the Sacred, Blood of the Damned
This program is a full rewrite of Gabriel Knight 3 running in a new cross-platform engine. In many ways, it looks and feels like the original game, though you can surely spot differences, improvements, and deficiencies.

This README will help you get the game up and running so you can play!

## Data Files
When you first run the game, you will get an error message about missing data files. In order to play the game, you will need to obtain a copy of GK3 (available on Steam or GOG) to get the data files. The data files are all Barn (.brn) asset bundles and all video files (.bik and .avi) in the game's `Data` directory.

On Windows and Linux, copy the data files to the `Data` folder in the exe directory. On Mac, copy them to `Gabriel Knight 3.app/Contents/Resources/Data`.

The [Gabriel Knight 3 demo](https://archive.org/details/GabrielKnight3BloodOfTheSacredBloodOfTheDamnedDemo) is also supported. Simply copy `Gk3demo.brn` into the data folder. This is a way you can get a  taste of the experience without purchasing the full game.

## Config Options
The file `GK3.ini` in the game's root directory provides some options for changing how the game functions. If interesting, take a look - all options are present and documented. Just uncomment an option to have it read by the game.

## Developer Mode
You can open the developer console using the `~` key, which allows you to enter commands in the Sheep language. For example, try `SetLocation("LBY")` or `PrintString("Hello World")`. Check out the GitHub wiki for a list of all Sheep API functions that can be called.

Use the `Tab` key to toggle developer tools mode - this gives you access to a menu for enabling debug visualizations or viewing scene/asset info. More tools and features will be added to this mode over time.

## Troubleshooting
This section contains notes or tips to help troubleshoot common issues when running the game.

### Mac Gatekeeper Issues
The Mac app is not codesigned (purely to avoid the $99 yearly fee), so your Mac will warn you that GK3 is potentially malicious software and refuse to open it. You will need to jump through a few hoops to open the app. See [this Apple support page](https://support.apple.com/en-us/102445) for guidance.

### Linux Performance Issues
If the game exhibits poor performance on your Linux system, ensure that you've installed the latest drivers for your graphics card. The default driver provided out-of-the-box by Linux is likely not very effective for 3D rendering. In my testing, the default driver was only able to achieve 15-20 FPS. After installing the latest Nvidia driver, I was able to achieve 60 FPS consistently.