# Mapping Demo Application

The demo application serves three purposes:

- show off the algorithms implemented in the library
- facilitate exploration of interaction techniques for the design of mappings
- integration-test the algorithms in a realistic usage scenario

The application is implemented using SDL and OpenGL, and is tested under Arch
Linux. There are plans to eventually support a web-based version of the app,
but development is still in very early stages so the overhead of supporting
more than one platform is not yet justified.

# Overview

At this time I have only implemented interpolators, so the demo only
demonstrates inteprolators. As additional types of algorithms are introduced,
they may be integrated into the same demo, or a second application may be
developed.

The interpolator demo requires the following functionality:

- visualize the topology of the active interpolator
- select different visualisation modes
- select a different active interpolator
- edit the uniform parameters of the active interpolator

- create demonstration points 
    - e.g. by clicking an on screen button, 
    - hitting a key
    - or double click
- edit source point positions by dragging
- edit sound / colour with knobs or sliders
    - drag or turn mousewheel
- remove demonstration points
    - e.g. by selecting and then hitting a button or key
    - or double click
- edit multiple points simultaneously
    - adjust from current values

- join source points and presets to form demonstrations
- edit demonstration attributes of active interpolator
- remove demonstration associations

- query the interpolator by dragging anywhere
- lock the demonstrations to prevent accidentally dragging them
- switch between edit (unlocked) and performance (locked) modes

The user interaction elements could be summarised:

- drag dots on the display (for editing source points and making queries)
- edit scalar parameters (synthesis parameters, demo attributes, interpolator uniforms)
- form a selection of demonstrations or sliders
- select from several options (visualization mode, active interpolator, edit mode)

For educational purposes, let's consider how the design could be acheieved using
different achitectural paradigms.

# Entity-Component-System

Systems:
- demonstration visualiser
    + position, colour, radius, selected
- attribute visualiser
    + bounding box
    + active interpolator number of attributes, names of attributes, list of
      selected demonstrations, colours, presets, attribute values
    - pure functional; doesn't need to edit anything
- uniform visualiser
    + bounding box
    + active interpolator number of uniforms, names of uniforms, uniform values
    - pure functional; doesn't need to edit anything
- selection system
    + hitboxes, associated selection groups, mouse info (up/down), selection mode
    - links selection groups
- slider system
    + hitboxes, selection lists, mouse info (down position, velocity), 
      slide mappings
    - sets attribute values based on product of slide mapping coeff and mouse velocity
- demonstration gardener
    + demonstration positions, mouse info
    - removes and plants demonstrations
- undo system
    + commands
    - keeps a command stack and associates the top of stack with undo/redo
      buttons and key shortcuts
- command system (keyboard shortcut system)
    + hitboxes, commands, mouse clicks (keyboard shortcuts)
    - calls commands when clicks on hitboxes (shortcuts pressed)
- command button visualiser
    + hit/bounding boxes, icons, various colours, mouse state
    - draw buttons including hover, press, release state
- 

- demonstration entity
    + position component
    + colour component
    + preset component
    + (optional) selection group component
    + attribute component(s)
- knobentity
    + bounding box component
    + (optional) selection group component
    + background colour component
    + foreground colour component
    + selected colour component
    + hovered colour component
- radio entity
    + bounding box component
    + option components
    + active option component
- mouse/touch input entity
    + position
    + velocity
    + z (up or down)
    + dz (first difference of up or down)
    + down position
    + up position

# Observer / Listener Pattern (Callbacks)

Define three interaction widgets and some sub-system managers:

- source vector widget
- knob widget
- radio switch widget
- visualisation widget
- selection manager
- query playback manager

**Source Vector**: This represents a demonstration or query.  
*Listens to*: Mouse. 
*Observed by*: Demonstrations, selection manager.
*On render*: Draw a ring centered on your position.
*On click*: Send a request to the selection manager to be added to the selection.
*On dragged*: Adjust position; send update to listeners.
*Data members*: 
- source vector: position
- bool: selected
- colour: inactive
- colour: active
- scalar: radius

**Knob Widget**: A changeable scalar parameter.
*Listens to*: Mouse. Keyboard?
*Observed by*: Demonstrations, interpolators, selection manager.
*On render*: Draw a nice knob with your current value.
*On mouse down*: Check for hit. On hit, request selection.
*On dragged*: Adjust value; send update to listeners.
*Data members*:
- scalar: value
- bool: selected
- colour: inactive
- colour: active
- rectangle: bounding box

**Radio Switch Widget**: Select from several alternatives.
*Listens to*: Mouse. Keyboard?
*Observed by*: Query player, visualizer, selection manager?
*On render*: Draw several buttons
*On click*: Update current selection, send on-update event to listeners.
*Data members*:
- enum: current selection
- enum: list of possible selections
- list of strings: selection names
- list of icon textures: selection icons active
- ditto: inactive
- rectangle: bounding box

**Visualisation Widget**: Show interpolator topology
*Listens to*: Radio switch
*On render*: Draw visualisation
*Data members*:
- enum: active interpolator
- enum: visualization mode

**Selection Manager**: Keep track of selected objects and forward mouse drags
*Listens to*: Selectable widgets, mouse, keyboard
*Observed by*: Selectable widgets
*On mouse down*: If no modifier, deselect.
*On select request*: If shift, add range to selection. Elif control, add to
    selection. Else set selection. Set grabbed.
*On mouse move*: If mousedown, send mouse move to selected widgets.
*On mouse up*: Unset mousedown.
*Data members*:
- bool: grabbed
- list of selectable widgets: selected objects
- bools: key modifier state
Note that selection manager implicitly needs to handle mouse down events before
any selectable widgets.

There are numerous problems. How do you set up the deep chains of callbacks?
Who is in charge of keeping track of state and widgets? Who is allowed to do
which things?

- User clicks on a different active interpolator in the active interpolator radio selector
- Radio sends notification to..? Who?
    - Current sliders need to be destroyed, or have their callbacks
      de-registered and registered to new active interpolator, or hidden?
      New sliders need to be created, or re-activated. How do they get the
      handle for the selection manager in order to register?
    - Visualiser needs to switch to new interpolator
    - Query player needs to switch to the new interpolator

So just about every widget needs to seperately keep track of the active interpolator?

How about dragging a slider?

- Update value
- Value needs to sent to the selected demonstration(s)
- So actually the slider doesn't even have a single value; it should have a
  range of possible values based on how many things are selected, or it should
  visualize multiple values based on the things that are selected.
- So does the slider send updates to the selections? Or does the selection
  manager? Does the slider need to get updates at all or should it actually be
  a client of the selected things? If so, who is in charge of registering
  that slider with those things?

