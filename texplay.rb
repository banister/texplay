# (C) John Mair 2009, under the MIT licence

begin
    require 'rubygems'
rescue LoadError
end

# include gosu first
require 'gosu'

module Gosu
    class Image
        class << self
            alias_method :original_new_old, :new

            def new(*args, &block)

                obj = original_new_old(*args, &block)
                
                # keep track of window instance
                obj.instance_variable_set(:@__window__, args.first)

                obj
            end
        end

        def __window__
            @__window__
        end
    end
end

module TexPlay
    TEXPLAY_VERSION = "0.1.9.0 BETA"

    def self.on_setup(&block)
        raise "need a block" if !block
        
        @__init_procs__ ||= []
        @__init_procs__.push(block)
    end

    def self.setup(receiver)
        if @__init_procs__ then
            @__init_procs__.each do |init_proc|
                receiver.instance_eval(&init_proc)
            end
        end
    end

    module Colors
        Red = [1, 0, 0, 1]
        Green = [0, 1, 0, 1]
        Blue = [0, 0, 1, 1]
        Black = [0, 0, 0, 1]
        White = [1, 1, 1, 1]
        Grey = [0.5, 0.5, 0.5, 0.5]
        Alpha = [0, 0, 0, 0]
        Purple = [1, 0, 1, 1]
        Yellow = [1, 1, 0, 1]
        Cyan = [0, 1, 1, 1]
        Orange = [1, 0.5, 0, 1]
        Brown = [0.39, 0.26, 0.13, 1]
        Turquoise = [1, 0.6, 0.8, 1]
        Tyrian = [0.4, 0.007, 0.235, 1]
    end
    include Colors
end

# bring in user-defined extensions to TexPlay
require 'ctexplay'
require 'texplay-contrib'

# monkey patching the Gosu::Image class to add image manipulation functionality
module Gosu
    class Image

        # bring in the TexPlay image manipulation methods
        include TexPlay
        
        class << self 
            alias_method :original_new, :new
            
            def new(*args, &block)

                # invoke old behaviour
                obj = original_new(*args, &block)

                # refresh the TexPlay image cache
                if obj.width <= (TexPlay::TP_MAX_QUAD_SIZE - 2) &&
                        obj.height <= (TexPlay::TP_MAX_QUAD_SIZE - 2) && obj.quad_cached? then
                    
                    obj.refresh_cache
                end

                # run custom setup
                TexPlay::setup(obj)

                # return the new image
                obj
            end
        end
    end
end

# a bug in ruby 1.8.6 rb_eval_string() means i must define this here (rather than in texplay.c)

if RUBY_VERSION =~ /1.8/ then
    class Proc  
        def __context__
            eval('self', self.binding)
        end
    end
end




