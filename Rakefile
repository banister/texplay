direc = File.dirname(__FILE__)
dlext = Config::CONFIG['DLEXT']
project_name = "texplay"

require 'rake/clean'
require 'rubygems/package_task'
require "#{direc}/lib/#{project_name}/version"

CLOBBER.include("**/*.#{dlext}", "**/*~", "**/*#*", "**/*.log", "**/*.o")
CLEAN.include("ext/**/*.#{dlext}", "ext/**/*.log", "ext/**/*.o", "ext/**/*~",
              "ext/**/*#*", "ext/**/*.obj", "ext/**/*.def", "ext/**/*.pdb")

def apply_spec_defaults(s)
  s.name = "texplay"
  s.summary = "TexPlay is a light-weight image manipulation framework for Ruby and Gosu"
  s.version = TexPlay::VERSION
  s.date = Time.now.strftime '%Y-%m-%d'
  s.author = "John Mair (banisterfiend)"
  s.email = 'jrmair@gmail.com'
  s.description = s.summary
  s.require_path = 'lib'
  s.add_dependency("gosu",">=0.7.25")
  s.add_development_dependency("bacon",">=1.1.0")
  s.homepage = "http://banisterfiend.wordpress.com/2008/08/23/texplay-an-image-manipulation-tool-for-ruby-and-gosu/"
  s.has_rdoc = 'yard'
  s.files =  Dir["Rakefile", "README.markdown", "CHANGELOG", 
                      "lib/**/*.rb", "ext/**/extconf.rb", "ext/**/*.h", "ext/**/*.c",
                      "examples/*.rb", "examples/media/*", "test/*.rb", "live/*rb", ".gemtest"]
end

task :test do
  sh "bacon -k #{direc}/test/texplay_spec.rb"
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
    s.extensions = ["ext/#{project_name}/extconf.rb"]
  end

  Rake::GemPackageTask.new(spec) do |pkg|
    pkg.need_zip = false
    pkg.need_tar = false
  end
end

directories = ["#{direc}/lib/1.8", "#{direc}/lib/1.9"]
directories.each { |d| directory d }

desc "build the 1.8 and 1.9 binaries from source and copy to lib/"
task :compile => directories do
  build_for = proc do |pik_ver, ver|
    sh %{ \
          c:\\devkit\\devkitvars.bat && \
          pik #{pik_ver} && \
          ruby extconf.rb && \
          make clean && \
          make && \
          cp *.so #{direc}/lib/#{ver} \
        }
  end
  
  chdir("#{direc}/ext/#{project_name}") do
    build_for.call("187", "1.8")
    build_for.call("192", "1.9")
  end
end

desc "build all platform gems at once"
task :gems => [:clean, :rmgems, "mingw32:gem", "mswin32:gem", "ruby:gem"]

desc "remove all platform gems"
task :rmgems => ["ruby:clobber_package"]

desc "build and push latest gems"
task :pushgems => :gems do
  chdir("#{direc}/pkg") do
    Dir["*.gem"].each do |gemfile|
      sh "gem push #{gemfile}"
    end
  end
end

