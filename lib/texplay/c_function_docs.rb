# -*- coding: iso-8859-1 -*-
# This code is never run; it is just here so we can document the C functions.


module TexPlay
  # Draw a bezier curve.
  #
  # @param (see #polyline)
  # @option (see #polyline)
  # @return [Gosu::Image]
  def bezier(points, options = {})
  end

  # Draw a circle.
  #
  # @note :thickness not implemented for circle.
  #
  # @param [Number] center_x Horizontal center.
  # @param [Number] center_y Vertical center.
  # @param [Number] radius Radius.
  # @option (see #set_pixel)
  # @option options [Boolean] :fill (false) Whether to fill in the shape.
  # @option options [Boolean] :filled (false) Synonym for +:fill+
  # @return [Gosu::Image]
  def circle(center_x, center_y, radius, options = {})
  end

  # Make a copy of the image.
  #
  # @return [Gosu::Image] Deep copy of the image.
  def clone()
  end

  # (see #clone)
  def dup()
  end

  # Iterate through every pixel of the image, allowing modification of each pixels.
  #
  # @yield [color, x, y] Colour and position (arity == 3).
  # @yield [color] Colour only (arity == 1).
  # @yieldparam [Array<Float>] color RGBA colour array. This can be modified to affect the image directly.
  # @yieldparam [Integer] x X position of the pixel.
  # @yieldparam [Integer] y Y position of the pixel.
  def each(&block)
  end

  # Perform a Flood Fill at a given position.
  #
  # @param [Number] x
  # @param [Number] y
  # @option options [Gosu::Color, Array<Float>, Symbol] :color (:white) Colour to apply to the drawing action.
  # @option options [Proc, Hash] :color_control Effect to apply to the drawing action.
  # @return [Gosu::Image]
  def fill(x, y, options = {})
  end

  # Force synchronisation of the image (send from RAM to VRAM).
  #
  # @param [Array<Number>] rectangle Area to synchronise.
  # @return [Gosu::Image]
  def force_sync(rectangle = [0, 0, width, height])
  end

  # Get the colour of an individual pixel.
  #
  # @param [Number] x Horizontal position.
  # @param [Number] y Vertical position.
  # @return [Array<Float>] 4-element, RGBA colour array.
  def get_pixel(x, y)
  end

  # @param [Number] x1 X position of start of line.
  # @param [Number] y1 Y position of start of line.
  # @param [Number] x2 X position of end of line.
  # @param [Number] y2 Y position of end of line.
  # @option (see #set_pixel)
  # @option options [Number] :thickness (1) Thickness of the line.
  # @option options [Hash] :trace Trace
  # @return [Gosu::Image]
  def line(x1, y1, x2, y2, options = {})
  end

  # Draw an n-sided polygon.
  #
  # @param (see #circle)
  # @param [Integer] sides Number of sides for the polygon.
  # @option (see #circle)
  # @return [Gosu::Image]
  def ngon(center_x, center_y, radius, sides, options = {})
  end

  # Offset all future drawing actions.
  #
  # The offset is non-stacking.
  #
  # Can also use +offset(:default)+ to reset offset to (0, 0).
  #
  # @param [Number] x
  # @param [Number] y
  # @return [Gosu::Image]
  def offset(x, y)
  end

  # Perform drawing operations in the context of the image using _lazy_ syncing.
  #
  # The paint block may look like an ordinary instance_eval, but it is not. The problem that plagues the serious use of
  # instance_eval: namely the inability to use local instance variables in the block does not affect the Paint Block.
  # This means that code like the following is possible in TexPlay (but impossible with an instance_eval)
  #
  # How does it work? TexPlay uses an alternative to instance_eval known as gen_eval (http://github.com/banister/gen_eval/tree/master)
  # Another peculiarity of paint blocks is how they sync the drawing actions (syncing will be discussed in greater depth
  # later on). The drawing actions in a Paint Block are not sync’d until the very end of the Paint Block and are then
  # sync'd to video memory all in one go (This style of syncing is called _lazy_ syncing in TexPlay)
  #
  # @example Drawing to the image within a paint block
  #   image1.paint do
  #     circle 10, 10, 20, :color => :green
  #     bezier [0, 0, 10, 10, 50, 0], :closed => true
  #   end
  #
  # @example Showing gen_eval behaviour (can access ivars within the paint block)
  #   @x = 20
  #   @y = 30
  #   my_image.paint do
  #      circle @x, @y, 20
  #   end
  #
  # @option options [Symbol] :sync_mode (:lazy_sync) One of +:lazy_sync+, +:no_sync+ or +:eager_sync+
  # @yield Block evaluated as the image.
  # @return [Gosu::Image]
  def paint(options = {}, &block)
  end

  # Draw a series of connected lines.
  #
  # @param [Array<Number, TPPoint>] points Series of points (either [x1, y1, x2, y2, ...] or [point1, point2]).
  # @option (see #rect)
  # @option options [Boolean] :closed Whether the last point is linked to the first.
  # @return [Gosu::Image]
  def polyline(points, options = {})
  end

  # Draw a rectangle.
  #
  # @param [Number] x1 Top of the rectangle.
  # @param [Number] y1 Left of the rectangle.
  # @param [Number] x2 Right of the rectangle.
  # @param [Number] y2 Bottom of the rectangle.
  # @option (see #circle)
  # @option options [Number] :thickness (1) Thickness of the line.
  # @return [Gosu::Image]
  def rect(x1, y1, x2, y2, options = {})
  end
  alias_method :box, :rect

  # Set the colour of an individual pixel.
  #
  # @param [Number] x
  # @param [Number] y
  # @option (see #fill)
  # @option options [Boolean] :alpha_blend (false) Alpha-blend instead of overwrite.
  # @option options [Gosu::Image] :texture Texture to use instead of a flat colour.
  # @option options [Symbol] :mode (:copy) Drawing mode to use.
  # @return [Gosu::Image]
  def set_pixel(x, y, options = {})
  end
  alias_method :pixel, :set_pixel

  # Copy an image into another image.
  #
  # @param [Number] x Horizontal position to splice at.
  # @param [Number] y Vertical position to splice at.
  # @param [Gosu::Image] source Image to copy from.
  # @option options [Gosu::Color, Array<Float>, Symbol] :chroma_key Colour to treat as transparent (only other colour pixels will be spliced).
  # @option options [Gosu::Color, Array<Float>, Symbol] :chroma_key_not Colour to copy (other colours won't be spliced).
  # @option options [Array<Float>] :crop ([0, 0, source.width - 1, source.height - 1]) Area of the source image to splice.
  # @return [Gosu::Image]
  def splice(x, y, source, options = {})
  end
  alias_method :composite, :splice

  # Generate binary data from the image.
  #
  # @note This method is usually overridden by the default Gosu::Image#to_blob.
  # @note This method causes a segmentation fault in Windows.
  # @return [String] Raw binary data (blob) in RGBA byte order.
  def to_blob
  end
end
