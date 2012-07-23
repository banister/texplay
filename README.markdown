![Alt text](https://dl.getdropbox.com/u/239375/texplay.png)

*an image manipulation tool for ruby and gosu*

INSTRUCTIONS 
============

**TexPlay version 0.4.3**

[Read The Documentation](http://banisterfiend.wordpress.com/2008/08/23/texplay-an-image-manipulation-tool-for-ruby-and-gosu/)

Gem installation:

+ sudo gem install texplay

How to build the gems? (maintainers only)

+ Install rake-compiler (http://github.com/luislavena/rake-compiler)
+ Install 1.9.1 and 1.8.6 mingw ruby versions (instructions above)
+ Type (on Linux/OSX to build fat Win32 binary): `rake cross native gem RUBY_CC_VERSION=1.8.6:1.9.1`

+ Upload new gems to rubyforge and gemcutter.

How to build from source?    
+ rake compile

OR

+ rake19 compile (assuming this is the name of your 1.9.1 version of rake)

**NB:** be sure to run the version of rake that corresponds to the ruby version you wish to use! on my system I use rake19 for ruby 1.9.1!

If all goes well, run the example programs:

+ cd examples
+ ruby example_melt.rb
+ ruby example_simple.rb
+ ..etc

+ like any gosu application, gosu.so must be in the current directory (or the gosu gem installed) when running the examples.

Third party
-----------

+ (Windows only) [Freeglut](http://freeglut.sourceforge.net/) [Freeglut Copyright]

---
Enjoy!
