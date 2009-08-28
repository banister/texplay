require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 769, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/empty2.png")
        @tp = Gosu::Image.new(self, "#{Common::MEDIA}/texplay.png")


        # put a border on the image
        @img.rect 0,0, @img.width - 1, @img.height - 1

        # it can be annoying having to specify a bunch of :hash_arguments for every action
        # here is how you specify common hash args that all actions will use (in the same image)

        # all actions that respond to 'thickness' use a thickness of 8 pixels
        # also set the color to random
        @img.set_options :thickness => 8, :color => :rand

        @img.rect 100, 100, 200, 200, :fill => false

        # NOTE: for ngon, the parameters are as follows: x, y, radius, num_sides
        @img.ngon 400, 400, 40, 3

        # NOTE: the defaults can also be overidden:
        @img.ngon 400, 200, 90, 6, :thickness => 1

        # you can also delete the defaults
        @img.delete_options

        # this action will no longer have any default values
        @img.ngon 200, 400, 90, 10
    end
    
    def draw
        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
