require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 768, false, 20)
        @img = TexPlay::create_blank_image(self, 500, 500)

        # put a border on the image
        @img.rect 0,0, @img.width - 1, @img.height - 1

        points = []

        # NOTE: TexPlay also accepts points. a 'point' is any object that responds to 'x' or 'y'
        # NOTE: current maximum points for a bezier is 13
        (0..@img.width + 100).step(50) { |x|
            p = TexPlay::TPPoint.new
            p.x = x
            p.y = @img.height * rand

            points << p
        }

        # making the bezier 
        @img.bezier points, :color => :red

        # NOTE: can 'close' a bezier curve too (as with polylines)
    end
    
    def draw

        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
