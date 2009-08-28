
$dlext = Config::CONFIG['DLEXT']

$make_program = if RUBY_PLATFORM =~ /win/ 
                    "nmake"
                else
                    "make"
                end

task :default => [:build]

desc "Remove files whose names ends with a tilde"
task :delete_backups do
    files = Dir['*~']
    rm(files, :verbose => true) unless files.empty?
end

desc "Build TexPlay"
task :build => :clean do
    puts "(2) building Makefile..."
    Dir.chdir("./src/")
    ruby "extconf.rb"
    puts "(3) building ctexplay.#{$dlext}..."
    sh "#{$make_program}"
    puts "(4) copying ctexplay.#{$dlext} to current directory..."
    cp "ctexplay.#{$dlext}", "../" 
    puts "(5) ...done!"
end

task :clean do
    puts "(1) clearing away old files..."
    Dir.chdir("src/")
    files = Dir['*.{so,o,log}']
    rm(files, :verbose => true) unless files.empty?
    Dir.chdir("../")
end

    
