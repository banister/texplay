require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(500, 500, false, 20)
        @img = TexPlay::create_blank_image(self, 1022, 800)
        @tp = Gosu::Image.new(self, "#{Common::MEDIA}/texplay.png")
        @gosu = Gosu::Image.new(self, "#{Common::MEDIA}/sand1.png")

        points = []

        # NOTE: TexPlay also accepts points. a 'point' is any object that responds to 'x' or y
        # NOTE: current maximum points for a bezier is 13
        (0..@img.width + 90).step(185) { |x|
            p = TexPlay::TPPoint.new
            p.x = x
            p.y = @img.height * rand

            points << p
        }

        @img.bezier points

        # NOTE: the :texture hash argument works on ALL drawing actions; not just fills
        @img.fill 300, 650, :texture => @gosu

        # let's demonstrate by drawing a circle using the gosu.png texture
        # NOTE: :texture even works on lines, boxes, polylines, beziers etc. 
        @img.circle 400, 50, 40, :fill => true, :texture => @tp
    end
    
    def draw
        @img.draw 0, 0,1
    end
    
end


w = W.new
w.show
        
