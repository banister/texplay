require 'rubygems'
require 'test/unit'
require 'shoulda'
rquire 'common'
require 'gosu'
require 'texplay'

class MyWindow < Gosu::Window
    def initialize
        super(1024, 768, false, 20)
    end
end

W = MyWindow.new

class MyTest < Test::Unit::TestCase
    
    context "TexPlay Bug Eradicator" do
        setup do
            @img = Gosu::Image.new(W, "#{Common::MEDIA}/empty2.png")
            @gosu = Gosu::Image.new(W, "#{Common::MEDIA}/gosu.png")
            @points = []
        end
        
        should "not have gaps in polyline from 2nd to 3rd point" do
            @img.polyline 0, 0, 40, 40, 38, 1024
        end

        should "not freeze when using floodfill and a color_control block" do
            # rectangle to fill
            @img.rect 40, 40, 100, 100

            # works fine WITHOUT color_control block, wtf?
            @img.fill 90, 90, :color_control => proc { |c, x, y| :red }
        end

        # this problem is due to inability to calculate factorials > 15
        should "beziers should not CLUSTER around top left hand corner when num random points > 15" do
            15.times { 
                p = [(@img.width * rand()), (@img.height * rand())]
                @points += p
            }
            @img.paint { 
                bezier @points
            }
        end

        # fixed
        should "make composite actions imitate primitives, i.e random color does not trickle to sub actions" do
            # comprised of filled circles
            @img.line 300, 300, 500, 500, :color => :random, :thickness => 5

            # comprised of lines
            @img.circle 40, 300, 10, :color => :random, :fill => true
        end

        should "this crashes the stack after a little while" do
            l = ->() { 2 * @img.width * rand - @img.width / 2 }

            points = []
            10.times {
                points += [l.(), l.()]
            }

            @img.bezier points, :closed => true
            
            @img.fill l.(), l.(), :color => :random#, :scan => true
            
        end

        should "fill crashes when using alpha_blend => true" do
            @img.fill 250, 487, :texture => @tp, :scan => true, :alpha_blend => true
        end

        # FIXED ! simply did not initialize TexPlay::TPPoint, duh!
        should "not ERROR about finding ivar @turtle_pos, SUPPOSED to be eval'd in context of @img" do
            @img.paint {
                forward(50, true, :color => :red)
                turn(@length)
            }
        end
    end

    context "effects" do
        setup do
            @length = 1
            @img = Gosu::Image.new(W, "empty2.png")
        end

        
        should "this is just a cool effect using turtle and random fill" do
            @img.paint { 
                forward(@length, true, :color => :red)
                turn(170)
                @length += 5
            }
            @img.fill (@img.width * rand), (@img.height * rand), :color => :random
        end

        should "another cool effect using turtle" do
            @length = 0
            @img.paint {
                forward(20, true, :color => red)
                turn(@length)
                @length += 10
            }
        end
        
        should "draw a circle with a rough edge utilizing color_control block and :none (no color)" do
            @img.circle 200,200, 100,
            :color_control => proc { |c,x,y|
                @img.pixel(x + (rand(4) - 2), y + (rand(4) - 2)) 
                :none   # this ensures that the 'actual' circle isn't drawn, instead its coordinates are
                        # simply utilized to draw a ragged edge of another circle (using @img.pixel)
            }
        end

        context "examples" do
            setup do
                @img = Gosu::Image.new(W, "empty2.png")
                @tp = Gosu::Image.new(W, "texplay.png")
            end

            should "draw closed polyline" do
                @img.polyline 0,0 , 40, 40, 400, 300, 100, 20, :closed => true, :thickness => 1
            end

            should "texture fill an area using :texture hash arg" do
                @img.polyline 30, 30, 100, 100, 200, 76, 300, 9, 50, 200, :color => :random, :closed => true
                @img.fill 42, 70, :texture => @tp
            end

            should "texture fill an area using color_control" do
                @img.polyline 30, 30, 100, 100, 200, 76, 300, 9, 50, 200, :color => :random, :closed => true
                @img.fill 42, 70, :color_control => proc { |c, x, y|
                    @tp.get_pixel(x % @tp.width, y % @tp.height)
                }
            end

            should "average the color between the texture image and the background image" do
                @img.rect 10, 10, 100, 100, :filled => true, :texture => @tp,
                :color_control => proc { |c1, c2|
                    c1[0] = (c1[0] + c2[0] / 2 )
                    c1[1] = (c1[1] + c2[1] / 2 )
                    c1[2] = (c1[2] + c2[2] / 2 )
                    c1[3] = 1
                    c1
                }
            end

            should "force a texture to alpha blend even when it has a full opacity" do
                @img.circle 100, 100, 50, :alpha_blend => true, :fill => true, :texture => @gosu,
                :color_control => proc { |c, c1|
                    c1[3] = 0.1
                    c1
                }
            end

            should "draw lines with thickness, using color_control block" do
                x1 = @img.width * rand
                y1 = @img.height * rand
                x2 = @img.width * rand
                y2 = @img.height * rand

                # thickness of lines
                t = 30
                
                @img.paint {
                    line x1, y1, x2, y2, :color_control => proc {  |c, x, y|

                        # vectors
                        vx = x2 - x1
                        vy = y2 - y1

                        # unit vectors
                        uvx = vx / Math::hypot(vx, vy)
                        uvy = vy / Math::hypot(vx, vy)

                        # draw lines at perpendicular
                        line x, y, x - (uvy*t), y + (uvx*t), :color => :red

                        # original line is white
                        :white
                    }
                }

                
            end

            should "do an alternative way to thick draw lines" do
                x1 = @img.width * rand
                y1 = @img.height * rand
                x2 = @img.width * rand
                y2 = @img.height * rand

                t = 30

                vx = x2 - x1
                vy = y2 - y1

                uvx = vx / Math::hypot(vx, vy)
                uvy = vy / Math::hypot(vx, vy)
                
                @img.paint {
                    line x1, y1, x2, y2
                    t.times { |i|
                        line x1 - (uvy * i), y1 + (uvx * i), x2 - (uvy * i), y2 + (uvx*t), :color => :random
                    }
                }

            end

            


            context "bezier context" do
                setup do
                    @points = []
                    @p = TexPlay::TPPoint.new
                    @p.x, @p.y = 0, 0
                    @points << @p
                    (0..@img.width).step(80) do |x|
                        @p = TexPlay::TPPoint.new
                        @p.x = x
                        @p.y = rand(@img.height) * 2 - @img.height / 2
                        @points << @p
                    end
                    @p = TexPlay::TPPoint.new
                    @p.x, @p.y = @img.width, 0
                    @points << @p
                end

                should "draw a bezier curve that spans the image width" do
                    @img.bezier(*@points)
                end
                
            end
        end        
    end

