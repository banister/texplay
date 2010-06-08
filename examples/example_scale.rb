require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(500, 500, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/logo.png")
        @img2 = TexPlay::create_blank_image(self, 500, 500)

        @img2.splice_and_scale @img, 0, 50, :factor => 2
        @img2.splice @img, 0, 200 
    end
    
    def draw
        x = @img.width * rand
        y = @img.height * rand

        @img2.draw 100, 100,1
    end
    
end


w = W.new
w.show
        
