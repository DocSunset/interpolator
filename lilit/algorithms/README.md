# Mappings

In the design of digital musical instruments, mappings are the designed
connections that define the interactive behaviour of the instrument.
Typically, a physical control interface (e.g. MIDI device, motion catpure
system, or custom DIY controller) is connected to a sound synthesis system;
this connection is the mapping stage of the instrument, and can be considered
as the third part of the instrument next to the controller and the synthesizer.
More generally, regardless of the final output medium (sound, light, video,
water vapour, robot control, or whatever), interactive media systems used by
artists working with technology are almost always composed from various modular
subsystems.  The various connections between these systems play a role in
determining the overall behavior of the aggregate whole, and may be treated as
mappings in the same way as the connection from controller to synthesizer in
the digital musical instrument.

It is recognized that mappings are crucial in the design of these systems,
having wide-reaching influence on their overall quality.  Because of the
recognized importance of mappings in digital musical instruments, various
algorithms have been presented in the literature to facilitate the development
and creative exploration of mappings.  These algorithms can be broadly
categorized in various ways.  In terms of their internal implementation, my own
review of the literature suggests the following main categories, listed now in
order of their prevalence in the literature:

- [preset interpolation](interpolators/README.md)
- neural networks
- temporal models
- [linear and affine transforms (matrices)](matrices/README.md)
- gaussian mixture models
- and other techniques

On the other hand, the techniques presented in the literature can also be
characterized by the interaction modalities that they support. These can
be roughly categorized like this:

- manual patching and tweaking
- point-to-preset association
- design by demonstration
- and high-level design strategies

No matter how you group the tools, the largest bucket is the catch-all "other"
category; this reflects the idiosyncratic texture of the music technology
research community.  The more distinct categories reflect prominent themes,
rather than characterizing the overall approach seen in the literature, which
is best described as "rich and varied", in a way that resists clear
categorization.
