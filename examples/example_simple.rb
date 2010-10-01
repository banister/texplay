$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'


class W < Gosu::Window
    def initialize
        super(500, 500, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/empty2.png")
        @gosu = Gosu::Image.new(self, "#{Common::MEDIA}/gosu.png")

        # put a border on the image
        @img.rect 0,0, @img.width - 1, @img.height - 1, :color => 0xffff00ff, :fill => true

        # perform some e drawing actions 
        @img.line 0,0, @img.width - 1, @img.height - 1, :color => Gosu::Color::AQUA
        @img.circle 400, 100, 40, :fill => true, :color => [rand, rand, rand, 1]
      @img.rect 200, 300, 300, 400, :fill => true, :color => :red, :source_ignore => [:green],
      :color_control => proc {
        rand(2) == 1 ? :red : :blue
      }

        @img.ngon 400,300,50, 5, :start_angle => 90
        @img.ngon 300,300,50, 5, :start_angle => 45

        # NOTE: chroma_key means NOT to splice in that color (pixels with that color are skipped)
        # (chroma_key_not does the opposite, it skips pixels that do NOT have that color)
        @img.splice @gosu, 210, 330,
      :dest_select => [:blue], :source_ignore => [:alpha, :green]

      @img.line 200, 300, 300, 400, :thickness => 5,
      :dest_select => :blue, :dest_ignore => :red

      puts (@img.get_pixel 2000, 310).inspect
      puts @img.get_pixel 2000, 310, :color_mode => :gosu
    end
    
    def draw
        @img.draw 100, 50,1
    end
    
end


w = W.new
w.show
        
