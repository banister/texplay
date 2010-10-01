$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'


class W < Gosu::Window
    def initialize
      super(500, 500, false, 20)
      @img = TexPlay.create_image(self, 50, 50, :color => Gosu::Color::BLUE)
      @img2 = TexPlay.create_image(self, 50, 50, :color => Gosu::Color::RED)

    end

    def update
  
    end
    
    def draw
        @img.draw 0, 0,1
      @img2.draw 100, 100,1
      if button_down?(Gosu::KbEscape)
#        self.flush
        @blob = self.to_blob(0,self.height - 70, 50, 50)
        if @blob
          @img3 = TexPlay.from_blob(self, @blob,50, 50 ) 
        end
      end

      @img3.draw rand(300), rand(300), 1 if @img3
      500000.times {}
    end
    
end


w = W.new
w.show
        
