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
    
    if RUBY_PLATFORM =~ /mingw/
        $CFLAGS += " -I/home/john/.rake-compiler/ruby/ruby-1.9.1-p243/include"
        $CFLAGS += " -I/home/john/.rake-compiler/ruby/ruby-1.8.6-p287/include"
        $CFLAGS += " -D FREEGLUT_STATIC"
    end
end

# 1.9 compatibility
$CFLAGS += " -DRUBY_19" if RUBY_VERSION =~ /1.9/

create_makefile('texplay')
