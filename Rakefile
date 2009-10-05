require 'rake/clean'
require 'rake/gempackagetask'
require 'rake/extensiontask'

TEXPLAY_VERSION = "0.2.5"

$dlext = Config::CONFIG['DLEXT']

CLEAN.include("ext/**/*.#{$dlext}", "ext/**/*.log", "ext/**/*.o", "ext/**/*~", "ext/**/*#*", "ext/**/*.obj", "ext/**/*.def", "ext/**/*.pdb")
CLOBBER.include("**/*.#{$dlext}", "**/*~", "**/*#*", "**/*.log", "**/*.o")

specification = Gem::Specification.new do |s|
    s.name = "texplay"
    s.summary = "TexPlay is a light-weight image manipulation framework for Ruby and Gosu"
    s.version = TEXPLAY_VERSION
    s.date = Time.now.strftime '%Y-%m-%d'
    s.author = "John Mair (banisterfiend)"
    s.email = 'jrmair@gmail.com'
    s.description = s.summary
    s.require_path = 'lib'
    s.add_dependency("gosu",">=0.7.14")
    s.platform = Gem::Platform::RUBY
    s.extensions = ["ext/texplay/extconf.rb"]
    s.homepage = "http://banisterfiend.wordpress.com/2008/08/23/texplay-an-image-manipulation-tool-for-ruby-and-gosu/"
    s.has_rdoc = false
    s.files =  ["Rakefile", "README.markdown", "CHANGELOG", 
                "lib/texplay.rb", "lib/texplay-contrib.rb"] +
        FileList["ext/**/extconf.rb", "ext/**/*.h", "ext/**/*.c", "examples/*.rb",
                 "examples/media/*"].to_a 


end

Rake::GemPackageTask.new(specification) do |package|
    package.need_zip = false
    package.need_tar = false
end

Rake::ExtensionTask.new('texplay', specification)  do |ext|
    ext.config_script = 'extconf.rb' 
    ext.cross_compile = true                
    ext.cross_platform = 'i386-mswin32'
end

# SELENE = '/home/john/ruby/projects/selene'
# desc "update selene's version of texplay"
# task :selene => ["#{SELENE}/lib/texplay.rb", "#{SELENE}/lib/texplay-contrib.rb",
#               "#{SELENE}/lib/ctexplay.so"] do
#     puts "...done!"
# end

# file "#{SELENE}/lib/texplay.rb" => "texplay.rb" do |t|
#     cp t.prerequisites.first, t.name, :verbose => true
# end

# file "#{SELENE}/lib/texplay-contrib.rb" => "texplay-contrib.rb" do |t|
#     cp t.prerequisites.first, t.name, :verbose => true
# end

# file "#{SELENE}/lib/ctexplay.#{$dlext}" => "ctexplay.#{$dlext}" do |t|
#     cp t.prerequisites.first, t.name, :verbose => true
# end
