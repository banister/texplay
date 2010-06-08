require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'


class W < Gosu::Window
    def initialize
      super(500, 500, false, 20)
      @img = TexPlay.create_image(self, 500, 500, :color => Gosu::Color::BLUE)
    end
    
    def draw
        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
