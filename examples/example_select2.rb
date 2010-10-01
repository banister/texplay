$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'
require 'benchmark'


class W < Gosu::Window
  def initialize
    super(500, 500, false, 20)
    @img = Gosu::Image.new(self, "#{Common::MEDIA}/sunset.png")
    @maria = Gosu::Image.new(self, "#{Common::MEDIA}/maria.png")
    @img.rect 0,0, @img.width - 1, @img.height - 1
    @img.splice @maria, 0, 0,  :tolerance => 0.70, :source_select => [:brown,:yellow], :crop => [200, 50, 500, 800]
    
  end
  
  def draw
    @img.draw 0, 0,1
  end
  
end


w = W.new
w.show

