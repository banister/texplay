require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'

class W < Gosu::Window
    def initialize
        super(500, 500, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/sunset.png")
        @img.rect 0,0, @img.width - 1, @img.height - 1

        # test the fluent interface
        @img.
            line(0, 0, 1024, 1024).
            circle(20, 20, 50).
            rect 30, 30, 100, 100
    end
    
    def draw
        x = @img.width * rand
        y = @img.height * rand
        
        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
