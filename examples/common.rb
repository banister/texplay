EXAMPLES_DIR = File.dirname(File.expand_path(__FILE__))

$LOAD_PATH.unshift File.expand_path(File.join(EXAMPLES_DIR, '..', 'lib'))

require 'gosu'
require 'texplay'

module Common
    MEDIA = File.join(EXAMPLES_DIR, 'media')
end


