require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'

class Window < Gosu::Window
  def initialize
    super(500, 500, false, 20)

    @image = TexPlay.create_image(self, 50, 50)

    from_x = 50
    from_y = 50
    to_x = 51
    to_y = 51

    x2, y2, color = @image.line from_x, from_y, to_x, to_y, :trace => { :while_color => :red }

    p x2
    p y2
    p color
  end
end

Window.new.show
