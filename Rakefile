begin
  require 'devkit' # Only on Windows.
rescue LoadError
end

require 'rake/extensiontask'

spec = Gem::Specification.load Dir['*.gemspec'].first

Gem::PackageTask.new spec do
end

Rake::ExtensionTask.new 'texplay', spec do |ext|
  RUBY_VERSION =~ /(\d+.\d+)/
  ext.lib_dir = "lib/texplay/#{$1}"
end
