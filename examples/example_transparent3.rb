$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'

class W < Gosu::Window
  def initialize
    super(400, 300, false, 20)
    @img = Gosu::Image.new(self, "#{Common::MEDIA}/object.png")

    @img.rect 0, 0, 200, 100, :color => :green, :fill => true, :dest_select => :transparent, :tolerance => 0.5
  end
  
  def draw
    @img.draw 100, 100, 1
  end
  
end


w = W.new
w.show
