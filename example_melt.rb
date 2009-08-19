require 'rubygems'
require 'gosu'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 769, false, 20)
        @img = Gosu::Image.new(self, "rose.bmp")
    end
    
    def draw
	
        x = @img.width * rand		
        y = (@img.height/2+ 100) * rand

        @img.splice @img, x, y + 1, :crop => [x, y, x + 200, y + 200]

        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
