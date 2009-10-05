require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(1024, 768, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/sunset.png")
        @img.rect 0,0, @img.width - 1, @img.height - 1

        # testing Gosu::Image#dup
        # adding an ivar
        @img.instance_variable_set(:@horse, :love)

        # adding a method on the singleton
        class << @img
            def little
                :little
            end
        end
        
        # clone the image.
        # NB #clone also copies singleton
        @bunk = @img.clone

        # should output :love
        puts @bunk.instance_variable_get(:@horse)

        # should output :little
        puts @bunk.little

        # add a red line to the copy to identify it
        #@bunk.line 0, 0, 1024, 1024, :color => :red

        @bunk.each(:region =>[200,200,350,350]) { |c,x,y|

            num_pixels = 0
            total = [0, 0, 0, 0]
            @bunk.circle x, y, 2,
            :color_control => proc { |v|
                if v
                    total[0] += v[0]
                    total[1] += v[1]
                    total[2] += v[2]
                    total[3] += v[3]

                    num_pixels += 1
                end
                :none
            }

            c[0] = total[0] / num_pixels.to_f
            c[1] = total[1] / num_pixels.to_f
            c[2] = total[2] / num_pixels.to_f
            c[3] = total[3] / num_pixels.to_f
        }

    end
    
    def draw
        x = @img.width * rand
        y = @img.height * rand

        @img.draw 100, 50,1
        @bunk.draw 500, 300,1
    end
    
end


w = W.new
w.show
        
