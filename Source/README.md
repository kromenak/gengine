# Source
All the engine's source code is contained in this folder and its subfolders. Though the subfolders are not "libraries" (strictly speaking), it's usually helpful to think of each one as a contained module. 

Large parts of this engine are entirely or relatively game-agnostic (e.g. Math, Primitives, Object Model, UI, etc) while some parts are extremely specific to GK3 (e.g. GAS, VertexAnimation, LocationManager, SceneInitFile, etc). And then there's some grey-area for "Adventure Game" genre-specific functionality (e.g. Actions, Walk/Navigation, Dialogue, etc). When dividing up engine content, think of it as a tower with "general game engine stuff" at the base, "adventure game stuff" in the middle, and "GK3-only stuff" at the top.

Here's a quick summary of subfolder:

## Assets
The asset subsystem. "AssetManager" provides mechanisms for loading assets from the disk or from GK3's archive "BRN" format. In-engine representations of assets inherit from the "Asset" class.

AssetManager currently references GK3-specific asset types, but generally this module could be game-agnostic with a bit of work.

Improvements:

- Currently can only read BRN format with single file extract mechanisms. Cannot yet fully extract a BRN or generate a new BRN from loose files.
- GK3 allows specifying additional asset search paths in an INI file - not yet implemented in this engine.

## Audio
Audio playback, both 2D and 3D, currently using FMOD. 

Some of these assets are game-agnostic (Audio, AudioManager, AudioListener) while others are maybe more GK3-specific (Soundtrack, SoundtrackPlayer). But generally, this should be considered a game-agnostic module.

Improvements:

- 3D positional sound kind of works, but a lot of the game systems do not yet play 3D sounds correctly. For example, audio played via Animations currently is all 2D.

## Debug
Debug systems, such as in-scene visualization of primitives (axes, line segments, planes, rects, triangles, AABBs) and the in-game console system. All of this is pretty game-agnostic.

Improvements:

- Cannot yet debug draw a sphere (harder than it seems haha).
- Mini-console is not yet implemented.
- GK3 has an entire "construction" mode accessible via the TAB key. Not yet implemented.

## GK3
This subfolder is meant to contain all GK3-specific functionality. These are classes that exist specifically to emulate GK3 gameplay based on how the original game was implemented.

Some stuff in this folder is arguably useful outside of just GK3 (e.g. the Action and Noun-Verb-Case system, inventory system). Perhaps it'll make sense to isolate it someday into some sort of "adventure game framework".

Some stuff in this folder seems like it should be game-agnostic (e.g. Scene, Heading). Again, perhaps something to look at and resolve when it makes sense to do so.

## Input
Mouse and keyboard input. Also includes a way to get text input (currently used by console system). Totally game-agnostic.

Improvements:

- Very dependent on SDL right now, and actually exposes keyboard key checks using SDL Scancodes (ideally, this'd be hidden somehow).
- Key states (down, pressed) are kind of ambiguous. I think using "leading edge" and "trailing edge" terminology could help.

## IO
Helpers for reading/writing data from/to any external source (the disk primarily, but could be network or something). Mostly game-agnostic.

Improvements:

- Currently no facility to write text files (text files can be read using IniParser).
- I believe GK3 calls the IniParser a "ConfigParser" or similar. GK3 assets are pretty inconsistent about the format of their text files, but I'm guessing they managed to have a single parser that could handle them all.
- A big and complicated IO-related system that needs to be tackled at some point is the save/load system.

## Math
Low-level math objects (Vectors, Matrices, Quaternions). Game-agnostic.

## Object Model
This engine uses an object model similar to Unity and Unreal Engine. To populate the game world, you create "Actors" to which you attach "Components". You can create many different types of components by inheriting from the Component class. Unlike Unity (and like Unreal), you CAN inherit from Actor if desired. Totally game-agnostic.

Transform component allows an Actor to be positioned, rotated, scaled, and parented (just like Unity). RectTransform is primarily used by UI, and is very very similar to Unity's version of this component.

## Platform
The goal of this module is to hide platform-specific logic behind platform-agnostic interfaces. "Platform.h" converts platform-specific defines into platform-agnostic defines. FileSystem allows accessing paths, directories, and files. SystemUtil retrieves system info (e.g. computer name or user name). Game-agnostic.

Improvements:

- "Atomics" is meant to define platform-agnostic built-in types of specific sizes. However, it isn't really used much yet - at some point, I'll need to go back through most of the game systems and sub these in!

## Primitives
Geometric primitives that are primarily used to represent simplified collision bounds for 3D objects. We can then perform intersection tests to see whether objects are colliding and take some action. Also provides support for things like "ray casting". Game-agnostic.

Improvements:

- Intersection tests only exist between certain primitives. For example, AABB-AABB intersection is supported, but AABB-Plane is not. I'm implementing on an as-needed basis.

## Rendering
Anything related to rendering. Some is quite low-level (meshes, vertex arrays, textures, colors, shaders, camera). Some are a bit higher-level (BSP, Skybox). "Renderer" is the main class that actually drives rendering. Mostly game-agnostic (some BSP stuff and the Model class are somewhat GK-specific).

## Reports
Essentially, this is a logging system. Allows any game subsystem to log to one or more "streams". Each stream is named and performs certain actions when data is sent to it (e.g. log to console, log to file, display OS popup, etc). Pulled directly from GK3's architecture as described in docs, but actually game-agnostic.

## Sheep
Sheep is GK3's custom scripting language. It consists of both a compiler (for taking a text-based script and compiling to bytecode) and a virtual machine (for executing bytecode). Despite being taken from GK3 specifically, this code is game-agnostic. The only thing that has some GK3-specific code is "SheepAPI", and it would be possible to split this into multiple files for "general" vs "GK3" stuff.

## UI
The UI system - similar in structure to Unity's UI system. Each UI screen is a "canvas", which contains one or more "widgets". Currently implemented widgets include Images, Labels, Buttons, and Text Inputs. Mostly game-agnostic; the Font and Cursor systems are maybe a bit GK3-specific.

Improvements:

- You may notice that font rendering looks a bit glitchy, depending on your display. Not sure why this is yet.

## Util
Utilities - my rule for inclusion here is usually "do I wish C++ had included this functionality?" If so, I stick it here.

## Video
Video playback via ffmpeg. Despite using ffmpeg, there's A TON of code that needs to be written to actually use it for video playback. All that code is in here (based heavily off of the sample ffplay code, but converted to C++). This code is game-agnostic, but ffmpeg was compiled specifically with GK3 in mind, so it only includes support for decoding BINK video files and certain AVI video files.

Improvements:

- This code includes some support for subtitle playback, but it is unfinished and not tested. GK3 does have subtitles during video playback, but the subtitles are played through the animation system.
- The Sierra logo video (which uses the cinepak codec) does not play back smoothly. Other videos seem fine though.
- When a video is playing, the 3D scene does not pause and any audio from the 3D scene mixes with the video's audio. To fix this, I probably need to implement the "layer" (i.e. game state) system.