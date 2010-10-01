$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'

class W < Gosu::Window
  def initialize
    super(500, 300, false, 20)
    @spritesheet = Gosu::Image.new(self, "body.png", false)
    @spritesheet.splice(Gosu::Image.new(self, "face.png", false), 0,0, :alpha_blend => true)
    @sprite_array = Gosu::Image::load_tiles(self, @spritesheet, 40, 80, false)
  end
  
  def draw
    @sprite_array[0].draw(200,200,0)
  end
  
end


w = W.new
w.show


# require 'rubygems'
# require 'gosu'
# require 'texplay'
# class Game_Window < Gosu::Window
# 	def initialize
# 		super(500, 400, false)
# 		self.caption = "Image Test"
# 	end

# 	def draw
# 	end
# end
