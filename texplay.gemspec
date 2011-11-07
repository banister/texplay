require File.dirname(__FILE__) + '/lib/texplay/version'

Gem::Specification.new do |s|
  s.name        = 'texplay'
  s.version = TexPlay::VERSION
  s.date = Time.now.strftime '%Y-%m-%d'
  s.summary = "TexPlay is a light-weight image manipulation framework for Ruby and Gosu"
  s.description = s.summary
  s.authors = ["John Mair (banisterfiend)"]
  s.email = ['jrmair@gmail.com']
  s.files =  Dir["Rakefile", "README.markdown", "CHANGELOG",
    "lib/**/*.rb", "ext/**/extconf.rb", "ext/**/*.h", "ext/**/*.c",
    "examples/*.rb", "examples/media/*", "test/*.rb", "live/*rb", ".gemtest"]
  s.has_rdoc = 'yard'
  s.homepage = "http://banisterfiend.wordpress.com/2008/08/23/texplay-an-image-manipulation-tool-for-ruby-and-gosu/"
  s.add_runtime_dependency("gosu",">=0.7.25")
  s.add_development_dependency("rspec",">=2.0.0")
  s.add_development_dependency("rake-compiler",">=0.7.9")
  s.require_path = 'lib'
  s.extensions = ['ext/texplay/extconf.rb']
end
