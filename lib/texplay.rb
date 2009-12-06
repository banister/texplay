# (C) John Mair 2009, under the MIT licence

begin
    require 'rubygems'
rescue LoadError
end

direc = File.dirname(__FILE__)

# include gosu first
require 'rbconfig'
require 'gosu'
require '#{direc}/texplay/version'

module TexPlay
    class << self
        def on_setup(&block)
            raise "need a block" if !block
            
            @__init_procs__ ||= []
            @__init_procs__.push(block)
        end

        def setup(receiver)
            if @__init_procs__ then
                @__init_procs__.each do |init_proc|
                    receiver.instance_eval(&init_proc)
                end
            end
        end

        def create_blank_image(window, width, height)
            Gosu::Image.new(window, EmptyImageStub.new(width, height))
        end

        alias_method :create_image, :create_blank_image
    end

    module Colors
        Red = [1, 0, 0, 1]
        Green = [0, 1, 0, 1]
        Blue = [0, 0, 1, 1]
        Black = [0, 0, 0, 1]
        White = [1, 1, 1, 1]
        Grey = [0.5, 0.5, 0.5, 1]
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

# credit to philomory for this class
class EmptyImageStub
    def initialize(w, h)
        @w, @h = w, h
    end
    
    def to_blob
        "\0" * @w * @h * 4
    end
    
    def rows
        @h
    end
    
    def columns
        @w
    end
end

# bring in user-defined extensions to TexPlay
direc = File.dirname(__FILE__)
dlext = Config::CONFIG['DLEXT']
begin
    if RUBY_VERSION && RUBY_VERSION =~ /1.9/
        require "#{direc}/1.9/texplay.#{dlext}"
    else
        require "#{direc}/1.8/texplay.#{dlext}"
    end
rescue LoadError => e
    require "#{direc}/texplay.#{dlext}"
end
    
require "#{direc}/texplay-contrib"

# monkey patching the Gosu::Image class to add image manipulation functionality
module Gosu
    class Image

        # bring in the TexPlay image manipulation methods
        include TexPlay

        attr_reader :__window__
        
        class << self 
            alias_method :original_new, :new
            
            def new(*args, &block)

                # invoke old behaviour
                obj = original_new(*args, &block)

                # refresh the TexPlay image cache
                if obj.width <= (TexPlay::TP_MAX_QUAD_SIZE) &&
                        obj.height <= (TexPlay::TP_MAX_QUAD_SIZE) && obj.quad_cached? then
                    
                    obj.refresh_cache
                end

                # run custom setup
                TexPlay::setup(obj)

                 obj.instance_variable_set(:@__window__, args.first)

                # return the new image
                obj
            end
        end
    end
end

# a bug in ruby 1.8.6 rb_eval_string() means i must define this here (rather than in texplay.c)
class Proc  
    def __context__
        eval('self', self.binding)
    end
end




