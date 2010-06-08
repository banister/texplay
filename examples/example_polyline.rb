require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(500, 500, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/empty2.png")

        # put a border on the image
        @img.rect 0,0, @img.width - 1, @img.height - 1

        points = []

        # NOTE: TexPlay also accepts points. a 'point' is any object that responds to 'x' or 'y'
        10.times {
            p = TexPlay::TPPoint.new
            p.x = @img.width * rand
            p.y = @img.height * rand

            points << p
        }


        # what if we want to turn a polyline into a polygon?
        @img.polyline points, :closed => true, :color => :blue
        @img.polyline points, :color => :red
    end
    
    def draw

        # NOTE: (when viewing) the blue line is the extra line added to close the polygon,
        # red lines are original polyline
        @img.draw 0, 0,1
    end
    
end


w = W.new
w.show
        
