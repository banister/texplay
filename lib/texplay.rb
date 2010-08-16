
# (C) John Mair 2009, under the MIT licence

begin
    require 'rubygems'
rescue LoadError
end

direc = File.dirname(__FILE__)

# include gosu first
require 'rbconfig'
require 'gosu'
require "#{direc}/texplay/version"

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

        def create_image(window, width, height, options={})
          options = {
            :color => :alpha,
            :caching => false,
          }.merge!(options)
          
          img = Gosu::Image.new(window, EmptyImageStub.new(width, height), :caching => options[:caching])
          img.rect 0, 0, img.width - 1, img.height - 1, :color => options[:color], :fill => true 

          img
        end

        alias_method :create_blank_image, :create_image

        def set_options(options = {})
            @options.merge!(options)
        end

        def get_options
            @options
        end

        # default values defined here
        def set_defaults
          @options = {
              :caching => true
          }
        end

        def init
            set_defaults
        end
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

    # extra instance methods defined in Ruby

    # clear an image (with an optional clear color)
    def clear(options = {})
      options = {
        :color => :alpha,
        :fill => true
      }.merge!(options)

      capture {
        rect 0, 0, width - 1, height - 1, options
      
        self
      }
    end
      
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

                options = args.last.is_a?(Hash) ? args.pop : {}
                # invoke old behaviour
                obj = original_new(*args, &block)

                prepare_image(obj, args.first, options)
            end
            
            alias_method :original_from_text, :from_text

            def from_text(*args, &block)

                options = args.last.is_a?(Hash) ? args.pop : {}
                # invoke old behaviour
                obj = original_from_text(*args, &block)

                prepare_image(obj, args.first, options)
            end

            def prepare_image(obj, window, options={})
                options = {
                  :caching => TexPlay.get_options[:caching]
                }.merge!(options)
                
                # refresh the TexPlay image cache
                if obj.width <= (TexPlay::TP_MAX_QUAD_SIZE) &&
                    obj.height <= (TexPlay::TP_MAX_QUAD_SIZE) && obj.quad_cached? then
                    obj.refresh_cache if options[:caching]
                end
                
                # run custom setup
                TexPlay.setup(obj)
              
                obj.instance_variable_set(:@__window__, window)

                obj
            end
            
            private :prepare_image
        end

        alias_method :rows, :height
        alias_method :columns, :width             
        end
end

# a bug in ruby 1.8.6 rb_eval_string() means i must define this here (rather than in texplay.c)
class Proc  
    def __context__
        eval('self', self.binding)
    end
end


# initialize TP (at the moment just setting some default settings)
TexPlay.init

