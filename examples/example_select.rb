require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'


class W < Gosu::Window
  def initialize
    super(500, 500, false, 20)
    @img = TexPlay.create_image(self, 500, 500)
    @img.rect 100, 100, 200, 300, :color => :red, :fill => true
    @img.rect 200, 100, 300, 300, :color_control => proc { [0, 0, 1 - (0.5 * rand), 1.0] }, :fill => true


    @img.line 0, 0, 500, 500, :dest_ignore => :red, :dest_select => :blue, :tolerance => 1

    @img.circle 200, 100, 50, :fill => true, :dest_select => [:red, :blue], :color => :transparent
    
  end
  
  def draw
    @img.draw 100, 50,1
  end
  
end


w = W.new
w.show

