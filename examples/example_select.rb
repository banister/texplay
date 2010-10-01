$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'


class W < Gosu::Window
    def initialize
      super(500, 500, false, 20)

      # draw a filled rect with left region blue, and right region red
      # at top and yellow at bottom
      @img = TexPlay.create_image(self, 500, 500, :color => Gosu::Color::BLUE)
      @img.rect 250,0, 500, 500, :color => :red, :fill => true
      @img.rect 250, 250, 500, 500, :color => :yellow, :fill => true

      # base rect is green on left and purple on right
      @base = TexPlay.create_image(self, 500, 500, :color => :green)
      @base.rect 250,0, 500, 500, :color => :purple, :fill => true

      # splice @img into @base, and select the yellow part of @img to
      # go into the purple part of @base
      # a combination of source_select
      # and dest_select - filtering both source and destination pixels
      @base.splice @img, 0, 0, :dest_select => :purple, :source_select => :yellow

    end
    
    def draw
      @base.draw 0, 0,1
    end
    
end


w = W.new
w.show
        
