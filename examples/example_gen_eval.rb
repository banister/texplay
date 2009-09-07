require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 768, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/empty2.png")

        @width = @img.width
        @height = @img.height

        # turn alpha blending and filling on
        @img.set_options :alpha_blend => true, :fill => true
    end
    
    def draw
        
        # Gen_eval lets us use local instance vars within the block
        # even though the block appears to be getting instance_eval'd
        # for more information see gen_eval.c and object2module.c
        @img.paint {
            rect @width * rand, @height * rand, @width * rand, @height * rand,
            :color => [rand, rand ,rand, rand]
        }  
        
        @img.draw 100, 50,1
    end
end

w = W.new
w.show
        
