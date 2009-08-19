require 'rubygems'
require 'gosu'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 769, false, 20)
        @img = Gosu::Image.new(self, "sunset.png")
        @img.rect 0,0, @img.width - 1, @img.height - 1

        @img.instance_variable_set(:@horse, :love)
        @bunk = @img.dup
        puts @bunk.instance_variable_get(:@horse)
    end
    
    def draw
        x = @img.width * rand
        y = @img.height * rand

        @img.line 0, 0, 1024, 1024
        @img.draw 100, 50,1
        @bunk.draw 500, 300,1
        @bunk.line 0, 0, 1024, 1024, :color => :red

        

        @bunk.pixel x, y, :color => :none
    end
    
end


w = W.new
w.show
        
