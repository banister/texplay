require 'mkmf'


# linux
if RUBY_PLATFORM =~ /linux/ then
    exit unless have_library("glut")
    
# macosx
elsif RUBY_PLATFORM =~ /darwin/
    $LDFLAGS +=  " -framework GLUT"
    $CPPFLAGS += " -I/System/Library/Frameworks/GLUT.framework/Headers"

# windows    
else
    exit unless have_library("glut32") && have_library("math")
end

create_makefile('src/ctexplay')
