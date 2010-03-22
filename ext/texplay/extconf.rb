require 'mkmf'


# linux
if RUBY_PLATFORM =~ /linux/ then
    exit unless have_library("glut")
    exit unless have_library("GL")
    
# macosx
elsif RUBY_PLATFORM =~ /darwin/
    $LDFLAGS +=  " -framework GLUT"
    $CFLAGS += " -I/System/Library/Frameworks/GLUT.framework/Headers"

# windows    
else
    
    exit unless have_library("freeglut_static") 
    exit unless have_library("opengl32")
    
end

# 1.9 compatibility
$CFLAGS += " -DRUBY_19" if RUBY_VERSION =~ /1.9/

# let's use c99
$CFLAGS += " -std=c99"

create_makefile('texplay')
