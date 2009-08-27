require 'rubygems'
require 'gosu'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 769, false, 20)
        @img = Gosu::Image.new(self, "sunset.png")
        @img.rect 0,0, @img.width - 1, @img.height - 1

        @img.instance_variable_set(:@horse, :love)
        class << @img
            def little
                :little
            end
        end
        @bunk = @img.clone
        puts @bunk.instance_variable_get(:@horse)
        puts @bunk.little
    end
    
    def draw
        x = @img.width * rand
        y = @img.height * rand

        @img.
            line(0, 0, 1024, 1024).
            circle(20, 20, 50).
            rect 30, 30, 100, 100
        
        @img.draw 100, 50,1
        @bunk.draw 500, 300,1
        @bunk.line 0, 0, 1024, 1024, :color => :red

        

        @bunk.pixel x, y, :color => :none
    end
    
end


w = W.new
w.show
        
