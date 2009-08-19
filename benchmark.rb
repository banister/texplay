# TexPlay test program

begin
    require 'rubygems'
rescue LoadError

end

require 'gosu'
require 'texplay'
require 'benchmark'

class MyWindow < Gosu::Window
    
    # set to true for visible output
    # note, benchmarks will not happen until
    # viewing window is closed
    SHOW = false

    def initialize
        super(1024, 768, false, 20)        
        @img = Gosu::Image.new(self,"texplay.png")
        @img2 = Gosu::Image.new(self,"texplay.png")
        @gosu = Gosu::Image.new(self, "gosu.png")
        @empty1 = Gosu::Image.new(self, "empty2.png")
        @empty2 = Gosu::Image.new(self, "empty2.png")
        @empty3 = Gosu::Image.new(self, "empty2.png")
        @width = @img.width
        @height = @img.height

        # default number of iterations 
        @repeat = 1000

        if ARGV.first.to_i != 0 then
            @repeat = ARGV.shift.to_i
        end

        if SHOW then
            class << self
                alias_method :draw, :_draw
            end
        end
    end
    
    def bench_all
        x1 = rand @width
        y1 = rand @height
        x2 = rand @width
        y2 = rand @height
        
        @img.paint { 
            color :random
            circle x1, y1, 10
            line x1, y1, x2, y2 
            rect x1, y1, x2 + 100, y2 + 100
            pixel x1, y1
        }
    end

    def bench_clear
        @img.paint { 
            clear
        }
    end

    def bench_circle
        x = rand @width
        y = rand @height
        r = 50
        
        @img.circle x, y, r
                
    end

    def bench_pixel
        @img.paint {
            color :random
            pixel (@img.width * rand), (@img.width * rand)
        }
    end

    def bench_splice
        @img.paint {
            splice(@img2, (@img.width * rand), (@img.height * rand))
        }
    end

    
    def bench_splice_same
        @img.paint {
            splice(@img, (@img.width * rand), (@img.height * rand))
        }
    end

    def bench_special_pixel
        @img.paint {
            self[(@img.width * rand), (@img.height * rand)] = :random
        }
    end
    
    def bench_line
        x1 = rand @width
        y1 = rand @height
        x2 = rand @width
        y2 = rand @height
        
        
        @img.line x1, y1, x2, y2
                
    end

    def bench_pline
        x1 = rand @width
        y1 = rand @height
        x2 = rand @width
        y2 = rand @height
        
        
        @img.paint {
            line x1, y1, x2, y2
        }
                
    end        

    def bench_box
        x1 = rand @width
        y1 = rand @height
        x2 = rand @width
        y2 = rand @height
        
        @img.paint {
            rect rand(@width),rand(@width),rand(@width),rand(@width)
        }        
    end

    def bench_anti_each
        0.upto(@img.width - 1) { |x|
            0.upto(@img.height - 1) { |y|
                clr = @img.get_pixel(x, y) 
                
                clr[0] += 1
                
                @img.color clr
                @img.pixel(x, y)
            }
        }
    end

    def bench_each
        @img.each { |v| v[0] = 1}
    end

    def bench_each2
        @img.each2 { |v| v[0] = 1}
    end

    def bench_gen_eval
        @img.paint {
            0.upto(1000) { 
                circle 100,100, 10
            }
        }
    end

    def bench_eager_sync
        @img.paint(:lazy_sync => false) {
#            1.upto(1000) { 
   #             color :blue
            ###                circle (10), (10), 10
            clear
                #circle (@img.width * rand), (@img.height * rand), 10
  #          }
        }
    end

    def bench_lazy_sync
        @img.paint(:lazy_sync => true) {
#            1.upto(1000) { 
 #               color :blue
            #              circle (10), (10), 10
            clear
                #circle (@img.width * rand), (@img.height * rand), 10
 #           }

        }
    end


    def bench_yield
        @img.paint { |c|
            0.upto(1000) { 
                c.circle 100,100, 10
            }
        }
    end

    def bench_instance_eval
        @img.instance_eval {
            0.upto(1000) { 
                circle 100,100, 10
            }
        }
    end

    def bench_glow_fill
        @empty2.fill 100,100
    end

    def bench_scan_fill_style
        @empty2.fill 100,100, :scan => true, :style => true, :color => :random
    end

    def bench_scan_fill
        @empty3.fill 100,100, :scan => true, :color => :random
    end

    def bench_iter_fill
        @empty3.fill 100,100, :iter => true
    end

    def bench_polyline_thick
        @empty2.polyline [100, 200, 0, 0, 150, 500, 500, 0], :closed => true, :texture => @tp, :thickness => 2,
        :alpha_blend => true, :sync_mode => :no_sync
    end

    def bench_polyline
        ps = []
        13.times {
            p = [@empty2.width * rand, @empty2.height * rand]
            ps += p
                }
       # ps = [110, 110, 210, 210]
        @empty2.polyline ps
    end

    def bench_bezier
        ps = []
        13.times {
            p = [@empty2.width * rand, @empty2.height * rand]
            ps += p
        }
        @empty2.bezier ps
    end

    def _draw
        bench_all
        @img.draw(200, 200, 0)
    end

    def do_benchmarks
        puts "performing #{@repeat} iterations..."

        bench_meths = ARGV
        
        if ARGV.empty? then
            bench_meths  = self.methods.select { |x| x =~ /bench/ }.map { |x| x.to_s.sub("bench_", "") }
        end
        
        Benchmark.bmbm do |v|
            bench_meths.each { |meth|
                v.report(meth) {  @repeat.times { self.send("bench_#{meth}") } }
            }
        end
    end
end

w = MyWindow.new

if MyWindow::SHOW
    w.show
end

w.do_benchmarks
