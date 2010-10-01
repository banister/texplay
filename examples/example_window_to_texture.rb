require 'common'
require 'gosu'
require 'texplay'


class W < Gosu::Window

  def render_to_texture(image, x, y)
    tex_name = image.gl_tex_info.tex_name
    xoffset = image.gl_tex_info.left * Gosu::MAX_TEXTURE_SIZE * Gosu::MAX_TEXTURE_SIZE
    yoffset = image.gl_tex_info.top * Gosu::MAX_TEXTURE_SIZE
    width = image.width
    height = image.height
    
    self.to_texture(tex_name, xoffset, yoffset, x, y, width, height)
  end

  def initialize
    super(500, 500, false, 20)
    @img = Gosu::Image.new(self, "#{Common::MEDIA}/maria.png")
    @img2 = TexPlay.create_image(self, 100, 100, :color => Gosu::Color::RED)

  end

  def update
    
  end
  
  def draw
    @img.draw 0, 0,1
    @img2.draw rand(100), rand(100),1
    @img3.draw rand(300), rand(300), 1 if @img3

    if button_down?(Gosu::KbEscape)
      #        self.flush
      @img3 = TexPlay.create_image(self, Gosu::MAX_TEXTURE_SIZE - 2, Gosu::MAX_TEXTURE_SIZE - 2, :color => :purple)

      render_to_texture(@img2, 0, self.height)



      # if @blob
      #   @img3 = TexPlay.from_blob(self, @blob,50, 50 ) 
      # end
    end

    500000.times {}
  end
  
end


w = W.new
w.show

