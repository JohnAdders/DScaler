========================================================================
PlugTest
========================================================================

This application uses the dtv files output from dTV by pressing the Shift+L key.

To use:

PlugTest sh000001.dtv DI_Greedy.dll sh000001.tif

This will load up the data from the sh000001.dtv file and pass it through the Greedy
deinterlacing plugin and then output a deinterlaced frame to the sh000001.tif file.

This program is intended to be used by dTV developers for debugging and comparing
video deinterlacing alogorithms using common input data.

This file is released under the terms og the GNU GPL for details see
http://www.gnu.org/copyleft/gpl.html


