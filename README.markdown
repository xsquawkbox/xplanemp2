# xplanemp2: Legacy-free Multiplayer Library for X-Plane

This is xplanemp2, a legacy-free rework of the multiplayer client code for 
X-Plane.

To clone this repository to your local drive, use:
```
git clone https://github.com/xsquawkbox/xplanemp2
```

## What's the Deal?

This codebase is the continuation of work I started in 2018 towards the original
next-generation XSquawkBox release.  Now that the AFV-Compatible XSB release is
out, I've started work in this area again.

When I started this work, the goal was to identify which features relied too 
heavily on 3D rendering passes and eliminate them, as well as attempting to
implement all the good practice notes that Ben & co have published over the
years.

Originally, it was going to be XP10 compatible, but when I revisited the 
library, I decided it'd be far more expedient to genuinely throw out all
the rarely used features that were going to be problematic and just focus on the
core functionality that must work right.

## Objectives

To provide common infrastructure to handle high volume multiplayer aircraft 
rendering, provide TCAS data, and other common convenience features that are
universal to most applications. 

It is a key requirement of this library that all features work uniformly over
Windows, macOS and Linux with all of the OpenGL, Vulkan and Metal renderers.
This is not to say that they must use the same codepaths, but they must behave 
the same.
 
## What's been removed compared to libxplanemp

* ACF rendering
  * initially removed it as it's not being used much anymore
  * more recently, it's been established as Not Compatible with Vulkan
  
* OBJ7 rendering
  * OBJ7 rendering has been a bad hack for a while now
  * Whilst it's potentially possible to port the renderer to Vulkan (ugh),
    it's not worth it and wouldn't cater for macOS (Metal) rendering and so
    it was removed completely.
    
* Aircraft monitor callbacks
  * I honestly don't know why these were even in there - if the plugin using
    xplanemp needs to know when xplanemp creates and destroys aircraft, it
    can hook it from it's own side.

* Labels in the 3D view.
  * There's no real way to make these work the way they did.  
  * It's not practical to render these in the 2D passes yet because, at least
    when I looked at it in 2018, we couldn't correlate 3d drawing passes to a 
    2D pass viewports rectangle.  The methods available worked for 
    single-monitor 3d rendering, but not multi-monitor 3d rendering. 
  * Even with the above solved, with no 3D pass to intercept on macOS, we can't
    get the view transformations for all enabled displays to do the labels.
  * This may be revisited later as the APIs improve and I have more time to
    attack it.
    
* Callback driven data API
  * As we're not reliably operating on a pull-during-render basis anymore, a 
    pull API no longer makes sense.   We now require clients to push updates
    every frame.
  * This includes the preferences stuff - we no longer take preference
    callbacks from the client, and instead use a static config structure.
    As xplanemp was typically statically linked to its consumer, this should be
    good enough. 
    
## What's new over libxplanemp

* Unified rendering framework.
  * When I was still trying to keep a foot in the XP10 world with this rework,
    I had implemented the renderer and internal state management such that
    new CSL types could be plugged in.  Whilst the old CSL renderers have
    been completely removed, the structure is still there and could potentially
    still support multiple object rendering paths.
    
* Map Rendering Support
  * Want to put aircraft on the map?  xplanemp2 now does that.
  * You need to provide a spritesheet for the map that has a rotatable aircraft
    glyph.  xplanemp2 will do the rest.
    
## What's Left to Do?

* TCAS needs an overhaul - current implementation should still work, but has
  been isolated to facilitate easier replacement when Laminar provides a better
  API.
    
## Release Status

I still consider this buggy code - whilst it's finally rendering well enough
to be used, and performance is OK, I have yet to do a serious profiling and 
debugging pass, so there's probably a few bugs left that leave callbacks enabled
past their nominal lifetime, or don't clean up long-lived dynamic resources
correctly.  These will probably get ironed out more as I get closer to a XSB3 
release.

Model-matching also needs a huge rewrite to stop wasting cycles during the
CSL search.

## Version Policy

Once an initial stable release has been reached, xplanemp2 will have clear
releases made in accordance with [Semantic Versioning](https://semver.org/).

The first stable version will be denoted 1.0.0.  We're just not there yet.

## Providing Changes

It's always preferential that an issue precedes any code change so any API 
changes can be discussed.

Once discussion has been completed, open a PR with a description of what the
change fixes or introduces referencing any relevant issues.

One feature or change, one PR.  Please do not make my job harder than it 
already is.

## License
```
Copyright (c) 2006-2013, Ben Supnik and Chris Serio
Copyright (c) 2015-2018,2020, Christopher Collins
Copyright (c) 2016-2017, Roland Winklmeier & Matthew Sutcliffe

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notices and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```
