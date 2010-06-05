require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'

class W < Gosu::Window
  def initialize
    super(500, 500, false, 20)

    @x1 = 10
    @x2 = 200
    @y1 = 10
    @y2 = 10

    @radius = 100.0
    @inner = 20.0
    @max_opacity = 0.5
    @max_opacity2 = 0.5

    penumbra = (@radius - @inner).to_i
    
    @img = Gosu::Image.new(self, "#{Common::MEDIA}/maria.png")
    #@img.rect(0, 0, @img.width - 1, @img.height - 1, :fill => true)

    @circ2 = TexPlay.create_image(self, @radius * 2, @radius * 2)

    @circ1 = TexPlay.create_image(self, @radius * 2, @radius * 2)

     
    shading_step = @max_opacity2.to_f / penumbra.to_f
    @radius.to_i.downto(@inner) { |r|
      @circ1.circle @radius, @radius, r, :color => [1, 0, 0, ((@radius - r) * shading_step)],
      :fill => true
    }

    @circ1.circle @radius, @radius, @inner, :color => [1, 0, 0, @max_opacity2], :fill => true
    
   shading_step = @max_opacity.to_f / penumbra.to_f
    @radius.to_i.downto(@inner) { |r|
      @circ2.circle @radius, @radius, r, :color => [0, 0, 1, ((@radius - r) * shading_step)],
      :fill => true
    }

    @circ2.circle @radius, @radius, @inner, :color => [0, 0, 1, @max_opacity], :fill => true
  end
  
  def draw
    @x1 += 1
    @y1 += 1

    @x2 -= 1
    @y2 += 1
    


    @img.draw 10, 10,1
    @circ1.draw @x1, @y1,1
    @circ2.draw @x2, @y1,1

    if button_down?(Gosu::KbEscape)
      IL.Enable(IL::ORIGIN_SET)
      IL.OriginFunc(IL::ORIGIN_UPPER_LEFT)
      #            screenshot.crop(0,0, 500, 500).save("screenshot.jpg").free
      exit
    end  
    
  end

  def update
  end
  
end


w = W.new
w.show

