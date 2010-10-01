$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'


class W < Gosu::Window
  def initialize
    super(500, 500, false, 20)
    @img = Gosu::Image.new(self, "#{Common::MEDIA}/gob.png")
    @img.rect 0, 0, @img.width - 1, @img.height - 1
    puts @img.line 1,1, 1, @img.height - 1, :trace => { :while_color => :alpha }
    
    
  end

  def draw
    @img.draw 50, 50,1
  end
end

w = W.new
w.show

