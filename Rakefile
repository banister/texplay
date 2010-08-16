
require 'rake/clean'
require 'rake/gempackagetask'
#require 'rake/extensiontask'

# get the texplay version
require 'lib/texplay/version'

$dlext = Config::CONFIG['DLEXT']

CLEAN.include("ext/**/*.#{$dlext}", "ext/**/*.log", "ext/**/*.o", "ext/**/*~", "ext/**/*#*", "ext/**/*.obj", "ext/**/*.def", "ext/**/*.pdb")
CLOBBER.include("**/*.#{$dlext}", "**/*~", "**/*#*", "**/*.log", "**/*.o")

# specification = Gem::Specification.new do |s|
#     s.name = "texplay"
#     s.summary = "TexPlay is a light-weight image manipulation framework for Ruby and Gosu"
#     s.version = TexPlay::VERSION
#     s.date = Time.now.strftime '%Y-%m-%d'
#     s.author = "John Mair (banisterfiend)"
#     s.email = 'jrmair@gmail.com'
#     s.description = s.summary
#     s.require_path = 'lib'
#     s.add_dependency("gosu",">=0.7.20")
#     s.platform = Gem::Platform::RUBY
#     s.homepage = "http://banisterfiend.wordpress.com/2008/08/23/texplay-an-image-manipulation-tool-for-ruby-and-gosu/"
#     s.has_rdoc = false

#     s.extensions = ["ext/texplay/extconf.rb"]
#     s.files =  ["Rakefile", "README.markdown", "CHANGELOG", 
#                 "lib/texplay.rb", "lib/texplay-contrib.rb", "lib/texplay/version.rb"] +
#         FileList["ext/**/extconf.rb", "ext/**/*.h", "ext/**/*.c", "examples/*.rb", "examples/media/*"].to_a 
# end

# Rake::ExtensionTask.new('texplay', specification)  do |ext|
#   ext.config_script = 'extconf.rb' 
#   ext.cross_compile = true                
#   ext.cross_platform = 'i386-mswin32'
#  ext.platform = 'i386-mswin32'
# end


#comment this when want to build normal gems.
#only have this code uncommented when building mswin32 and mingw32
#binary gems
specification = Gem::Specification.new do |s|
  s.name = "texplay"
  s.summary = "TexPlay is a light-weight image manipulation framework for Ruby and Gosu"
  s.version = TexPlay::VERSION
  s.date = Time.now.strftime '%Y-%m-%d'
  s.author = "John Mair (banisterfiend)"
  s.email = 'jrmair@gmail.com'
  s.description = s.summary
  s.require_path = 'lib'
  s.add_dependency("gosu",">=0.7.20")
  s.platform = 'i386-mswin32'
  s.homepage = "http://banisterfiend.wordpress.com/2008/08/23/texplay-an-image-manipulation-tool-for-ruby-and-gosu/"
  s.has_rdoc = false

  s.files =  ["Rakefile", "README.markdown", "CHANGELOG", 
              "lib/texplay.rb", "lib/texplay-contrib.rb", "lib/texplay/version.rb", "lib/1.8/texplay.so",
              "lib/1.9/texplay.so"] +
    FileList["examples/*.rb", "examples/media/*"].to_a
end

Rake::GemPackageTask.new(specification) do |package|
  package.need_zip = false
  package.need_tar = false
end
