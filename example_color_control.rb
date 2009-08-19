require 'rubygems'
require 'texplay'

class W < Gosu::Window
    def initialize
        super(1024, 769, false, 20)
        @img = Gosu::Image.new(self, "empty2.png")
        @tp = Gosu::Image.new(self, "texplay.png")
        @gosu = Gosu::Image.new(self, "gosu.png")

        # put a border on the image
        @img.rect 0,0, @img.width - 1, @img.height - 1

        # When using color_control the pixel the draw action is currently manipulating is yielded
        # to the proc. This gives you pixel-level control over the drawing.
        # (NOTE: the return value of the proc is used as the pixel color, so it should be a 4 element array or
        # a valid color symbol)
        
        # color_control accepts procs of 4 types:
        # arity of 1: just the destination pixel color is yielded
        # arity of 2: both the destination and the source pixel colors are yielded (in that order)
        # arity of 3: the destination pixel color is yielded along with the x and y coords of the current pixel
        # arity of 4: both the destination and the source pixel colours are yielded as well as the x and y vals

        # just drawing an area to fill
        @img.polyline [30, 30, 100, 100, 200, 76, 300, 9, 50, 200], :color => :random, :closed => true

        # below we are 'faking' a texture fill using color_control
        @img.fill 42, 70, :color_control => proc { |c, x, y|
            @tp.get_pixel(x % @tp.width, y % @tp.height)
        }

        # merging two images together
        @img.rect 100, 200, 400, 300, :fill => true, :texture => @gosu,
        :color_control => proc { |c1, c2, x, y|
            c1 = @tp.get_pixel(x % @tp.width, y % @tp.height)
            c1[0] = (c1[0] + c2[0]) / 2
            c1[1] = (c1[1] + c2[1]) / 2
            c1[2] = (c1[2] + c2[2]) / 2
            c1[3] = 1
            c1
        }

        # we can even use color_control just for the use of the (x, y) values.
        # here we simply use the x, y values to make our own circle with a rough edge
        @img.circle 200,400, 70,
        :color_control => proc { |c,x,y|
            @img.pixel(x + (rand(4) - 2), y + (rand(4) - 2)) 
            :none   # this ensures that the 'actual' circle isn't drawn, instead its coordinates are
                    # simply utilized to draw a ragged edge of another circle (using @img.pixel)
        }

        # this just fills a rect with random colours
        @img.rect 400, 400, 470, 470, :fill => true, :color_control => proc { |c| :rand }
    end

    def draw

        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
