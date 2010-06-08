require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(500, 500, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/empty2.png")
        @tp = Gosu::Image.new(self, "#{Common::MEDIA}/texplay.png")

        # put a border on the image
        @img.rect 0,0, @img.width - 1, @img.height - 1

        @img.rect 1, 1, @img.width - 2, @img.height - 2, :fill => true, :texture => @tp

        # NOTE: the current implementation of alpha blending is a bit tempermental, i need to think it
        # through a bit more...
        @img.rect 100, 100, 300, 300, :color => [1, 1, 1, 0.8], :alpha_blend => true, :fill => true
    end
    
    def draw
        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
