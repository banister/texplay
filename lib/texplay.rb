
# (C) John Mair 2009, under the MIT licence

direc = File.expand_path(File.dirname(__FILE__))

# include gosu first
require 'gosu'
require "#{direc}/texplay/version"
require 'texplay/texplay'

module TexPlay
  RENDER_CLEAR_COLOR = Gosu::Color.new(255, 0, 0, 0)

  class << self
    def on_setup(&block)
      raise "need a block" if !block
      @__init_procs__ ||= []
      @__init_procs__.push(block)
    end

    def setup(receiver)
      if @__init_procs__ 
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

      raise ArgumentError, "Height and width must be positive" if height <= 0 or width <= 0
      
      img = Gosu::Image.new(window, EmptyImageStub.new(width, height), :caching => options[:caching])

      # this should be a major speedup (avoids both a cache and a sync
      # if color is alpha (default)
      if options[:color] != :alpha
        img.rect 0, 0, img.width - 1, img.height - 1, :color => options[:color], :fill => true
      end

      img
    end

    alias_method :create_blank_image, :create_image

    # Image can be :tileable, but it will break if it is tileable AND gets modified after creation.
    def from_blob(window, blob_data, width, height, options={})
      options = {
        :caching => false,
        :tileable => false,
      }.merge!(options)

      raise ArgumentError, "Height and width must be positive (received #{width}x#{height})" if height <= 0 or width <= 0

      expected_size = height * width * 4
      if blob_data.size != expected_size
        raise ArgumentError, "Blob data is not of the correct size (expected #{expected_size} but received #{blob_data.size} bytes)"
      end

      Gosu::Image.new(window, ImageStub.new(blob_data, width, height), options[:tileable], :caching => options[:caching])
    end

    def set_options(options = {})
      @options.merge!(options)
    end

    def get_options
      @options
    end

    # default values defined here
    def set_defaults
      @options = {
        :caching => :lazy
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

  # Clear an image.
  #
  # @option options :color (:alpha) Colour of the image.
  # @return [Gosu::Image]
  def clear(options = {})
    options = {
      :color => :alpha,
      :fill => true
    }.merge!(options)

    rect 0, 0, width - 1, height - 1, options
  end

  # Used internally to create images from raw binary (blob) data (TexPlay::from_blob).
  #
  # This object duck-types an RMagick image (#rows, #columns, #to_blob), so that Gosu will import it.
  class ImageStub

    # @return [Integer]
    attr_reader :rows, :columns

    # The first pixel in the blob will be at the top left hand corner of the created image, since that is the orientation
    # of Gosu images.
    #
    # @param [String] blob_data Raw data string to import. Must be RGBA ordered, (4 * width * height) bytes in length.
    # @param [Integer] width Number of pixels wide.
    # @param [Integer] height Number of pixels high.
    def initialize(blob_data, width, height)
      @data, @columns, @rows = blob_data, width, height
    end

    # @return [String]
    def to_blob
      @data
    end
  end

  # Used internally to create blank images (red/blue/green/alpha all 0) (TexPlay::create_image).
  #
  # Credit to philomory for this class.
  class EmptyImageStub < ImageStub
    # @param width (see ImageStub#initialize)
    # @param height (see ImageStub#initialize)
    def initialize(width, height)
      raise ArgumentError if (width > TexPlay::TP_MAX_QUAD_SIZE || height > TexPlay::TP_MAX_QUAD_SIZE)
      super("\0" * (width * height * 4), width, height)
    end
  end
end

require "#{direc}/texplay-contrib"

# monkey patching the Gosu::Image class to add image manipulation functionality
module Gosu
  class Image

    # bring in the TexPlay image manipulation methods
    include TexPlay

    attr_reader :__window__
    protected :__window__
    
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

        caching_mode = options[:caching]

        # we can't manipulate large images, so skip them.
        if obj.width <= (TexPlay::TP_MAX_QUAD_SIZE) &&
            obj.height <= (TexPlay::TP_MAX_QUAD_SIZE)
          
          if caching_mode
            if caching_mode == :lazy

              # only cache if quad already cached (to refresh old data)
              # otherwise cache lazily at point of first TexPlay call
              obj.refresh_cache if obj.quad_cached?

            else
              
              # force a cache - this obviates the need for a
              # potentialy expensive runtime cache of the image by
              # moving the cache to load-time
              obj.refresh_cache
            end
          end
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

  class Window
    # Render directly into an existing image, optionally only to a specific region of that image.
    #
    # Since this operation utilises the window's back buffer, the image (or clipped area, if specified) cannot be larger than the
    # window itself. Larger images can be rendered to only in separate sections using :clip_to areas, each no larger
    # than the window).
    #
    # @note *Warning!* This operation will corrupt an area of the screen, at the bottom left corner, equal in size to the image rendered to (or the clipped area), so should be performed in #draw _before_ any other rendering.
    #
    # @note The final alpha of the image will be 255, regardless of what it started with or what is drawn onto it.
    #
    # @example
    #   class Gosu
    #     class Window
    #       def draw
    #         # Always render images before regular drawing to the screen.
    #         unless @rendered_image
    #           @rendered_image = TexPlay.create_image(self, 300, 300, :color => :blue)
    #           render_to_image(@rendered_image) do
    #             @an_image.draw 0, 0, 0
    #             @another_image.draw 130, 0, 0
    #             draw_line(0, 0, Color.new(255, 0, 0, 0), 100, 100, Color.new(255, 0, 0, 0), 0)
    #             @font.draw("Hello world!", 0, 50, 0)
    #           end
    #         end
    #
    #         # Perform regular screen rendering.
    #         @rendered_image.draw 0, 0
    #       end
    #     end
    #   end
    #
    #
    # @param [Gosu::Image] image Existing image to render onto.
    # @option options [Array<Integer>] :clip_to ([0, 0, image.width, image.height]) Area of the image to render into. This area cannot be larger than the window, though the image may be.
    # @return [Gosu::Image] The image that has been rendered to.
    # @yield to a block that renders to the image.
    def render_to_image(image, options = {})
      raise ArgumentError, "image parameter must be a Gosu::Image to be rendered to" unless image.is_a? Gosu::Image
      raise ArgumentError, "rendering block required" unless block_given?

      options = {
        :clip_to => [0, 0, image.width, image.height],
      }.merge! options

      texture_info = image.gl_tex_info
      tex_name = texture_info.tex_name
      x_offset = (texture_info.left * Gosu::MAX_TEXTURE_SIZE).to_i
      y_offset = (texture_info.top * Gosu::MAX_TEXTURE_SIZE).to_i

      raise ArgumentError, ":clip_to rectangle must contain exactly 4 elements" unless options[:clip_to].size == 4

      left, top, width, height = *(options[:clip_to].map {|n| n.to_i })

      raise ArgumentError, ":clip_to rectangle cannot be wider or taller than the window" unless width <= self.width and height <= self.height
      raise ArgumentError, ":clip_to rectangle width and height must be positive" unless width > 0 and height > 0

      right = left + width - 1
      bottom = top + height - 1

      unless (0...image.width).include? left and (0...image.width).include? right and
          (0...image.height).include? top and (0...image.height).include? bottom
        raise ArgumentError, ":clip_to rectangle out of bounds of the image"
      end

      # Since to_texture copies an inverted copy of the screen, what the user renders needs to be inverted first.
      scale(1, -1) do
        translate(-left, -top - self.height) do
          # TODO: Once Gosu is fixed, we can just pass width/height to clip_to
          clip_to(left, top, width, height) do
            # Draw over the background (which is assumed to be blank) with the original image texture,
            # to get us to the base image.
            image.draw(0, 0, 0)
            flush

            # Allow the user to overwrite the texture.
            yield
          end

          # Copy the modified texture back from the screen buffer to the image.
          to_texture(tex_name, x_offset + left, y_offset + top, 0, 0, width, height)

          # Clear the clipped zone to black again, ready for the regular screen drawing.
          # Quad can be a pixel out, so just make sure with a slightly larger shape.
          draw_quad(left - 2, top - 2, TexPlay::RENDER_CLEAR_COLOR,
                    right + 2, top - 2, TexPlay::RENDER_CLEAR_COLOR,
                    right + 2, bottom + 2, TexPlay::RENDER_CLEAR_COLOR,
                    left - 2, bottom + 2, TexPlay::RENDER_CLEAR_COLOR)
        end
      end

      image
    end
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

