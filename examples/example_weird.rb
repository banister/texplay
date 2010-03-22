require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
  def initialize
    super(1024, 768, false, 20)
    @img = Gosu::Image.new(self, "#{Common::MEDIA}/gob.png")
    @img.rect 0, 10, 300, 300, :color => :blue, :fill => true, :mode => :softlight
  end

  def draw
    @img.draw 100, 50,1
  end
end

w = W.new
w.show

