require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 768, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/sunset.png")
        puts @img.width
        puts @img.height
        puts @img.get_pixel(0,0).inspect
        puts @img.get_pixel(499,0).inspect
        puts @img.get_pixel(500,374).inspect
        puts @img.get_pixel(499,374).inspect
        puts @img.get_pixel(nil, 4)
    end
    
    def draw
        x = (@img.width - 100/2) * rand 
        y = (@img.height - 100/2) * rand 

        @img.splice @img, x, y + 1, :crop => [x, y, x + 100, y + 100]

        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
