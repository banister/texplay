require 'rake/clean'
require 'rake/gempackagetask'

$dlext = Config::CONFIG['DLEXT']

CLEAN.include("src/*.#{$dlext}", "src/*.log", "src/*.o", "src/*~", "src/*#*", "src/*.obj", "src/*.def", "src/*.pdb")
CLOBBER.include("**/*.#{$dlext}", "**/*~", "**/*#*", "**/*.log", "**/*.o")

$make_program = if RUBY_PLATFORM =~ /win/ 
                    "nmake"
                else
                    "make"
                end

task :default => [:build]

desc "Build TexPlay"
task :build => :clean do
    puts "(2) building Makefile..."
    chdir("./src/") do
        ruby "extconf.rb"
        puts "(3) building ctexplay.#{$dlext}..."
        sh "#{$make_program}"
        puts "(4) copying ctexplay.#{$dlext} to current directory..."
        cp "ctexplay.#{$dlext}", "../lib" , :verbose => true

        if RUBY_PLATFORM =~ /mswin/
            if RUBY_VERSION =~ /1.9/
                File.rename("../lib/ctexplay.#{$dlext}", "../lib/ctexplay.19.#{$dlext}")
            else
                File.rename("../lib/ctexplay.#{$dlext}", "../lib/ctexplay.18.#{$dlext}")
            end
        end
        puts "(5) ...done!"
    end
end

specification = Gem::Specification.new do |s|
    s.name = "texplay"
    s.summary = "TexPlay is a light-weight image manipulation framework for Ruby and Gosu"
    s.version = "0.2.2"
    s.date = "2009-09-09"
    s.author = "John Mair (banisterfiend)"
    s.email = 'jrmair@gmail.com'
    s.description = s.summary
    s.require_path = 'lib'
    s.add_dependency("gosu",">=0.7.14")
    s.homepage = "http://banisterfiend.wordpress.com/2008/08/23/texplay-an-image-manipulation-tool-for-ruby-and-gosu/"
    s.has_rdoc = false
    s.files =  ["Rakefile", "README.markdown", "CHANGELOG", "README1st",
                "lib/texplay.rb", "lib/texplay-contrib.rb"] +
        FileList["src/*", "examples/*.rb", "examples/media/*"].to_a 

    if RUBY_PLATFORM =~ /mswin/
        s.platform = Gem::Platform::CURRENT
        s.files += ["lib/ctexplay.18.so", "lib/ctexplay.19.so"]
        
    else
        s.platform = Gem::Platform::RUBY
        s.extensions = ["src/extconf.rb"]
    end
end
Rake::GemPackageTask.new(specification) do |package|
    package.need_zip = false
    package.need_tar = false
end

SELENE = '/home/john/ruby/projects/selene'
desc "update selene's version of texplay"
task :selene => ["#{SELENE}/lib/texplay.rb", "#{SELENE}/lib/texplay-contrib.rb",
              "#{SELENE}/lib/ctexplay.so"] do
    puts "...done!"
end

file "#{SELENE}/lib/texplay.rb" => "texplay.rb" do |t|
    cp t.prerequisites.first, t.name, :verbose => true
end

file "#{SELENE}/lib/texplay-contrib.rb" => "texplay-contrib.rb" do |t|
    cp t.prerequisites.first, t.name, :verbose => true
end

file "#{SELENE}/lib/ctexplay.#{$dlext}" => "ctexplay.#{$dlext}" do |t|
    cp t.prerequisites.first, t.name, :verbose => true
end
