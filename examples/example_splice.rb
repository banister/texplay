require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 768, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/texplay.png")
        @gosu = Gosu::Image.new(self, "#{Common::MEDIA}/gosu.png")

        @img.splice @gosu, 140,20, 
        :color_control => proc { |c1, c2, x, y|
            factor = 1 - (y - 25) / @gosu.height.to_f
            c2[0] *= factor
            c2[1] *= factor
            c2[2] *= factor
            c2
        }

        @img.splice @gosu, 50,20, :chroma_key => :alpha
    end

    def draw
        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
