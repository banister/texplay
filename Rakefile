$LOAD_PATH.unshift File.join(File.expand_path(__FILE__), '..')

require 'rake/clean'
require 'rake/gempackagetask'

# get the texplay version
require './lib/texplay/version'

dlext = Config::CONFIG['DLEXT']

CLEAN.include("ext/**/*.#{dlext}", "ext/**/*.log", "ext/**/*.o", "ext/**/*~", "ext/**/*#*", "ext/**/*.obj", "ext/**/*.def", "ext/**/*.pdb")
CLOBBER.include("**/*.#{dlext}", "**/*~", "**/*#*", "**/*.log", "**/*.o")

def apply_spec_defaults(s)
  s.name = "texplay"
  s.summary = "TexPlay is a light-weight image manipulation framework for Ruby and Gosu"
  s.version = TexPlay::VERSION
  s.date = Time.now.strftime '%Y-%m-%d'
  s.author = "John Mair (banisterfiend)"
  s.email = 'jrmair@gmail.com'
  s.description = s.summary
  s.require_path = 'lib'
  s.add_dependency("gosu",">=0.7.20")
  s.homepage = "http://banisterfiend.wordpress.com/2008/08/23/texplay-an-image-manipulation-tool-for-ruby-and-gosu/"
  s.has_rdoc = 'yard'
  s.files =  FileList["Rakefile", "README.markdown", "CHANGELOG", 
                      "lib/**/*.rb", "ext/**/extconf.rb", "ext/**/*.h", "ext/**/*.c",
                      "examples/*.rb", "examples/media/*", "spec/*.rb"].to_a 
end


[:mingw32, :mswin32].each do |v|
  namespace v do
    spec = Gem::Specification.new do |s|
      apply_spec_defaults(s)        
      s.platform = "i386-#{v}"
      s.files += FileList["lib/**/*.#{dlext}"].to_a
    end

    Rake::GemPackageTask.new(spec) do |pkg|
      pkg.need_zip = false
      pkg.need_tar = false
    end
  end
end

namespace :ruby do
  spec = Gem::Specification.new do |s|
    apply_spec_defaults(s)        
    s.platform = Gem::Platform::RUBY
    s.extensions = ["ext/texplay/extconf.rb"]
  end

  Rake::GemPackageTask.new(spec) do |pkg|
    pkg.need_zip = false
    pkg.need_tar = false
  end
end
  
desc "Run rspec 2.0"
task :rspec do
  system "rspec spec/**/*_spec.rb"
end

desc "Create yard docs"
task :doc do
  system "yard doc lib --output doc/yard --files doc/texplay_manual"
end  
