direc = File.dirname(__FILE__)

require "#{direc}/../texplay"

module TexPlay
  Win = Gosu::Window.new(100, 100, false)

  set_options :sync_mode => :no_sync

  class << self
    def load(name)
      Gosu::Image.new(Win, name)
    end
  
    alias_method :original_create_image, :create_image
    def create_image(width, height, options={})
      original_create_image(Win, width, height, options)
    end      
  end
end
