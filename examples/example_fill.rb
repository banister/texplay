require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 768, false, 20)
        @img = TexPlay::create_blank_image(self, 1000, 1000)
        #        @img = Gosu::Image.new(self, "#{Common::MEDIA}/empty2.png")
        @tp = Gosu::Image.new(self, "#{Common::MEDIA}/maria.png")
        @gosu = Gosu::Image.new(self, "#{Common::MEDIA}/sand1.png")

        points = []

        # NOTE: TexPlay also accepts points. a 'point' is any object that responds to 'x' or y
        # NOTE: current maximum points for a bezier is 13
        (0..@img.width + 90).step(85) { |x|
            p = TexPlay::TPPoint.new
            p.x = x
            p.y = @img.height * rand

            points << p
        }

        @img.bezier points


        # NOTE: the :texture hash argument works on ALL drawing actions; not just fills
        @img.fill 300, 400, :color => :red, :texture => @gosu
#        @img.circle 300, 400, 40

        # let's demonstrate by drawing a circle using the gosu.png texture
        # NOTE: :texture even works on lines, boxes, polylines, beziers etc. 
        @img.circle 400, 50, 40, :fill => true, :texture => @tp
        
    end
    
    def draw

        @img.draw 10, 10,1
    end
    
end


w = W.new
w.show
        
