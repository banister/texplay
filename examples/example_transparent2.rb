$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'

class W < Gosu::Window
  def initialize
    super(400, 300, false, 20)
    @img = TexPlay.create_image(self, 400, 300)
    @img.rect 0, 0, 200, 100, :color => [1,1,1,0], :fill => true
    @img.rect 200, 100, 400, 300, :color => [1,1,1,0.1], :fill => true
    

    @img.line 0, 120, 500, 120,
    :dest_select => :transparent, :color => :green, :tolerance => 0.1
  end
  
  def draw
    @img.draw 0, 0, 1
  end
  
end


w = W.new
w.show
