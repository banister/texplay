require 'rubygems'
require 'common'
require 'texplay'


class W < Gosu::Window
    def initialize
        super(500, 500, false, 20)
        @img = Gosu::Image.new(self, "#{Common::MEDIA}/empty2.png")

        # sets the 'global' color for all actions in this image
        @img.color :red

        # let's draw the circle without syncing it to gl
        @img.circle 100, 100, 50, :fill => true, :sync_mode => :no_sync

        # now let's sync half of it to gl
        @img.force_sync [100, 50, 150, 150]

        # let's draw some lazy shapes
        @img.set_options :sync_mode => :lazy_sync
        
        @img.ngon 200, 300, 40, 5, :color => :red
        @img.ngon 280, 300, 40, 6, :color => :green
        @img.ngon 360, 300, 40, 7, :color => :blue

        # now let's sync the lazy shapes to gl
        @img.paint
        
        # NOTE: the lazy drawing (above) is identical to using the following paint block
        # @img.paint {
        #     ngon 200, 300, 50, 5
        #     ...etc
        # }

        # end lazy drawing mode
        @img.delete_options
        
        # the default sync mode is eager_sync
        # in this mode actions are drawn and sync'd immediately
        # NOTE: TexPlay only syncs the part of the image that changed to gl
        @img.ngon 440, 300, 40, 8, :color => :tyrian

        # paint blocks can also accept a sync_mode parameter
        # NOTE: the line below will not be visible until you
        # explictly sync it to gl; probably using @img.force_sync
        @img.paint(:sync_mode => :no_sync) {
            line 0, 0, @img.width, @img.height
        }
    end
    
    def draw
        
        @img.draw 0, 0,1
    end
end

w = W.new
w.show
        
