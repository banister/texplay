require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(500, 500, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/sunset.png")
    end
    
    def draw
        x = (@img.width - 100/2) * rand 
        y = (@img.height - 100/2) * rand 

        @img.splice @img, x, y + 1, :crop => [x, y, x + 100, y + 100]

        @img.draw 0, 0,1
    end
    
end


w = W.new
w.show
        
