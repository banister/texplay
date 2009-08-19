require 'gosu'
require 'texplay'

class MyWindow < Gosu::Window
    def initialize
        super(1024, 768, false, 20)

        # image for texture filling
        @gosu = Gosu::Image.new(self,"gosu.png")

        # images for testing
        @unchanged = Gosu::Image.new(self,"texplay.png")  
        @simple = Gosu::Image.new(self,"texplay.png")  
        @polyline = Gosu::Image.new(self,"texplay.png") 
        @bezier = Gosu::Image.new(self,"texplay.png") 
        @flood_fill = Gosu::Image.new(self,"texplay.png") 

        # height and width
        @width = @simple.width
        @height = @simple.height

        # initializations
        setup_simple
        setup_polyline
        setup_bezier
        setup_flood_fill

    end

    def setup_simple
        @simple.line 0, 0, 1024, 1024
        @simple.rect 0,0, @width - 1, @height - 1
        @simple.circle @simple.width/2, @simple.height/2, 30
    end

    def setup_polyline
        @polyline.polyline [0, 0, @width / 2, @height - 1, @width - 1, 0]
    end

    def setup_bezier
        points = []
        10.times {
            point = []
            point[0] = @width * rand
            point[1] = @height * rand

            points += point
        }
        @bezier.bezier points
    end

    def setup_flood_fill
        @flood_fill.fill 100, 96, :color => :random
    end

    def draw

        @simple.draw(20, 10, 1)
        @polyline.draw(20, 210, 1)
        @bezier.draw(20, 410, 1)
        @flood_fill.draw(400, 10, 1)

        @polyline.paint

    end
end

w = MyWindow.new
w.show

    
