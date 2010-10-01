$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'


class W < Gosu::Window
    def initialize
        super(500, 500, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/texplay.png")
        @gosu = Gosu::Image.new(self, "#{Common::MEDIA}/gosu.png")

        @img.splice @gosu, 140,20, :alpha_blend => true, :mode => :default
        @img.rect 140,20, 160, 180, :color => [1,1,1,0.5], :alpha_blend => true, :fill => true

        @img.splice @gosu, 50,20, :chroma_key => :alpha
    end

    def draw
        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
