freeglut 2.8.0-1.mp for MinGW

This package contains freeglut import libraries, headers, and a Windows DLL,
allowing GLUT applications to be compiled on Windows using MinGW. Both static
and shared versions of the library are included.

For more information on freeglut, visit http://freeglut.sourceforge.net/.


Installation

Create a folder on your PC which is readable by all users, for example
“C:\Program Files\Common Files\MinGW\freeglut\” on a typical Windows system.
Copy the “lib\” and “include\” folders from this zip archive to that location.

The freeglut DLL should either be placed in the same folder as your application,
or can be installed in a system-wide folder which appears in your %PATH%
environment variable. On a 32 bit Windows system this is typically
“C:\Windows\System32\”, and on a 64 bit Windows system this is typically
“C:\Windows\SysWOW64\”.


Compiling Applications

If you want your application to be compatible with GLUT, you should
“#include <GL/glut.h>”. If you want to use freeglut specific extensions, you
should “#include <GL/freeglut.h>” instead.

Given a source file “test.c”, which you want to compile to an application
“test.exe” dynamically linking to the DLL, you can compile and link it with the
following commands (replacing the include and lib paths with the ones you
created above if necessary):

  gcc -c -o test.o test.c -I"C:\Program Files\Common Files\MinGW\freeglut\include"
  gcc -o test.exe test.o -L"C:\Program Files\Common Files\MinGW\freeglut\lib" -lfreeglut -lopengl32 -Wl,--subsystem,windows

Don’t forget to either include the freeglut DLL when distributing applications,
or provide your users with some method of obtaining it if they don’t already
have it!


Static Linking

To statically link the freeglut library into your application, it’s necessary to
define “FREEGLUT_STATIC” when compiling the object files. It’s also necessary to
link the static version of the freeglut library, along with the GDI and Windows
multimedia libraries which freeglut depends upon:

  gcc -c -o test.o test.c -D FREEGLUT_STATIC -I"C:\Program Files\Common Files\MinGW\freeglut\include"
  gcc -o test.exe test.o -L"C:\Program Files\Common Files\MinGW\freeglut\lib" -lfreeglut_static -lopengl32 -lwinmm -lgdi32 -Wl,--subsystem,windows

The “-Wl,--subsystem,windows” is needed in each case so that the application
builds as a Windows GUI application rather than a console application. If you
are using GLU functions you should also include “-lglu32” on the command line.

Please visit http://www.transmissionzero.co.uk/computing/using-glut-with-mingw/
for further information and usage.


Cross-Compilation

I’ve not covered the setup of freeglut for use in cross-compilation, i.e. when
building Windows freeglut applications using a Linux system. Setting freeglut up
with MinGW on other operating systems can be done following the instructions
above, except that the paths will be different.


Problems?

If you have problems using these packages (runtime errors etc.), please contact
me via http://www.transmissionzero.co.uk/contact/, providing as much detail as
you can. Please don’t complain to the freeglut guys unless you’re sure it’s a
freeglut bug, and have reproduced the issue after compiling freeglut from the
latest SVN version—if that’s still the case, I’m sure they would appreciate a
bug report or a patch.


Changelog

2012–01–15: Release 2.8.0-1.mp

  • First 2.8.0 MinGW release. I’ve built the package using MinGW, and the only
    change I’ve made is to the DLL version resource—I’ve changed the description
    so that my MinGW and MSVC builds are distinguishable from each other (and
    other builds) using Windows Explorer.


Martin Payne
2012–01–15

http://www.transmissionzero.co.uk/
