require 'rubygems'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 769, false, 20)
        @img = Gosu::Image.new(self, "empty2.png")

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

        @img.move_to(points.first.x, points.first.y)
        @img.bezier points, :color => :red, :color_control => proc { |c, x, y|
            if((x % 10) == 0) then
                @img.line_to(x, y + rand * 20 - 10)
            end
            :none
        }

        #@img.fill 100, 400, :color => :yellow
        
        # NOTE: can 'close' a bezier curve too (as with polylines)
    end
    
    def draw

        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
