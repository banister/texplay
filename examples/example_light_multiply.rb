require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'

class W < Gosu::Window
    def initialize
      super(500, 500, false, 20)
      @img = Gosu::Image.new(self, "#{Common::MEDIA}/sunset.png")
      @cover = TexPlay.create_image(self, @img.width, @img.height, :color => Gosu::Color.new(127, 127, 127, 127))
    end
    
    def draw
      @img.draw 0, 0, 1
      @cover.draw 0, 0, 1, 1, 1, Gosu::Color.new(0xffffffff), :multiply
    end
end

W.new.show
        
