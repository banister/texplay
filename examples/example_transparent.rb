require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'

class W < Gosu::Window
  def initialize
    super(400, 300, false, 20)
    @img = TexPlay.create_image(self, 400, 300)
    @img.rect 100, 100, 200, 200, :color => :red, :fill => true

    # Coloured transparent block with white border.
    @img.rect 200, 100, 300, 200, :color => [1, 1, 1, 0], :fill => true
    @img.rect 200, 100, 300, 200

    @img.line 0, 120, 500, 120, :dest_select => :transparent, :color => :green
    # @img.line 0, 140, 500, 140, :dest_select => :alpha  
    # @img.line 0, 160, 500, 160, :dest_select => :transparent # Should draw everywhere except the red block.
    # @img.line 0, 180, 500, 180, :dest_ignore => :transparent # Should draw only on the red block.       
  end
  
  def draw
    @img.draw 0, 0, 1
  end
  
end


w = W.new
w.show
