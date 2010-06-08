require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
  def initialize
    super(500, 500, false, 20)
    @img = Gosu::Image.new(self, "#{Common::MEDIA}/gob.png")
    @sun = Gosu::Image.new(self, "#{Common::MEDIA}/sunset.png")
    @img.splice @sun, 0, 0, :mode => :exclusion
  end

  def draw
    @img.draw 100, 50,1
  end
end

w = W.new
w.show

