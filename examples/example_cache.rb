$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'

class W < Gosu::Window
  def initialize
    super(400, 300, false, 20)
    TexPlay.set_options :caching => true
    @img = Gosu::Image.new(self, "#{Common::MEDIA}/object.png", :caching => false)

    @img.clear :color => :red, :dest_select => :transparent, :tolerance => 0.9 
  end
  
  def draw
    @img.draw 100, 100, 1
  end
  
end


w = W.new
w.show
