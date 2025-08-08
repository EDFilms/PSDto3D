Maya 2025
1) When using a 16 bit PSD, the PNGs saved out of Maya are corrupted with what looks like noise in the RGB. Probably this is a Maya problem and nothing to do with your plugin, but I thought it worth a mention.

2) The geometry created has its UVs offset to minus 1, while textures are at zero. (gave me a fright till I saw what was happening) This is easy enough to fix, but tedious if you have lots of layers.

3) The group created in Maya has its center at what I guess to be the lower left hand corner of the image file. Geometry created is then all to the right of center in the viewport. Not really a problem, but my first import showed an empty viewport which was a bit startling. The geometry created also has this offset center. Easy to fix but it would be nicer if created geometry had its center in its own plane.

no 3 isn't that important but I wondered if it might be connected to the strange UV offset.
