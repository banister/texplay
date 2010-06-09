require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
  def initialize
    super(500, 500, false, 20)
    @img = TexPlay.create_image(self, 500, 500).fill(3,3, :color => :alpha)
    @img.rect 100, 100, 300 , 300, :color => :blue, :fill => true
    puts (@img.line 498, 200, 600, 200, :trace => { :while_color => :blue })
  end

  def draw
    @img.draw 100, 50,1
  end
  
end


w = W.new
w.show

