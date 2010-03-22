require 'common'
require 'gosu'

Dragon = TexPlay::LSystem.new do
    rule "F" => "F"
    rule "X" => "X+YF+"
    rule "Y" => "-FX-Y"
    angle 90

    atom "FX"
end

Koch = TexPlay::LSystem.new do
    rule "F" => "F-F++F-F"

    angle 60

    atom "F"
end

Bush1 = TexPlay::LSystem.new do
    rule "F" => "F[-F]F[+F][F]"
    
    angle 20
    atom "F"
end

Bush2 = TexPlay::LSystem.new do
    rule "F" => "FF"
    rule "X" => "F[+X]F[-X]+X"
    
    angle 20
    atom "X"
end

Bush3 = TexPlay::LSystem.new do
    rule "F" => "FF"
    rule "X" => "F-[[X]+X]+F[+FX]-X"
    
    angle 22.5
    atom "X"
end



class W < Gosu::Window
    def initialize
        super(1024, 768, false, 20)
        @img = TexPlay::create_blank_image(self, 500, 500)
        @img.set_options :color => :rand
        @img.lsystem(400, 150, Koch, :order => 8, :line_length => 6)
        #@img.save("dragon.jpg")
    end

    def draw
        @img.draw(100,0,1)
    end
end

w = W.new
w.show
