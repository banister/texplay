require 'mkmf'

name = 'texplay/texplay'

dir_config name


# linux
if RUBY_PLATFORM =~ /linux/
  exit unless have_library("glut")
  exit unless have_library("GL")

# macosx
elsif RUBY_PLATFORM =~ /darwin/
  $LDFLAGS +=  " -framework GLUT"
  $CFLAGS += " -I/System/Library/Frameworks/GLUT.framework/Headers"

# windows
else
  freeglut_path = File.expand_path "vendor/freeglut", File.dirname(__FILE__)

  $CFLAGS << " -I#{File.join freeglut_path, "include"}"

  exit unless find_library "freeglut_static", 'main', File.join(freeglut_path, "lib")
  exit unless have_library "opengl32"

end

# Stop getting annoying warnings for valid C99 code.
$warnflags.gsub!('-Wdeclaration-after-statement', '') if $warnflags

# 1.9 compatibility
$CFLAGS += " -DRUBY_19" if RUBY_VERSION =~ /^1\.9/

# let's use c99
$CFLAGS += " -std=c99"

create_makefile name
