EXAMPLES_DIR = File.dirname(File.expand_path(__FILE__))

# Ensure that the texplay loaded is the one in this repository, not any gem installed.
$LOAD_PATH.unshift File.expand_path(File.join(EXAMPLES_DIR, '..', 'lib'))

begin
  require 'rubygems'
rescue LoadError => ex
end

require 'gosu'
require 'texplay'

module Common
    MEDIA = File.join(EXAMPLES_DIR, 'media')
end


