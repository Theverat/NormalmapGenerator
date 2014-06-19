Normalmap Generator

![screenshot](http://www.blendpolis.de/download/file.php?id=93358&mode=view)

todo:
- <b>(done)</b> image stack processing (whole folders, selection, <strike>after pattern?</strike>)
- more normalmap/edge detection filters (prewitt?)
- variable "working size" for these filters, e.g. 5x5 or 9x9
- add possibility to "blend" two normalmaps with different filter sizes for larger height differences while retaining small details
- same as above for displacementmaps
- better and easier control over channels for generating spec- and displacementmaps
- add contrast controls and similar stuff to spec- and dispmaps

concerning compiling/exporting:
- add support for CMake
- <b>(seems to work now)</b> figure out how the heck to include the needed .dll files for windows 8
- alternative: use static build
- add about page/link/whatever
