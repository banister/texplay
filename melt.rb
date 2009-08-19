require 'rubygems'
require 'gosu'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 769, false, 20)
        @img = Gosu::Image.new(self, "sunset.png")
#        @tp = Gosu::Image.new(self, "texplay.png")

        
    end
    
    def draw
        x = @img.width * rand
        y = @img.height * rand

        @img.splice @img, x, y + 1, :crop => [x, y, x + 100, y + 100]

        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
