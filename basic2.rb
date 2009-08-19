require 'gosu'
require 'texplay'

class Proc
    def __context__
        eval('self', self.binding)
    end
end

class W < Gosu::Window
    def initialize
        super(1024, 769, false, 20)
        @img = Gosu::Image.new(self, "texplay.png")
    end

    def trick
        @img.dup_eval { self }
    end

    def draw
        @img.paint {
            circle -5100, -9700, 20
            circle(@img.width * rand * 2 - @img.width, @img.height * rand * 2 - @img.height, 40)
        }
        @img.draw(100,100,1)
    end

    def img
        @img
    end
end

w = W.new
w.show
        
