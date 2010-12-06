direc = File.dirname(__FILE__)

require './bench_context'
require "#{direc}/../lib/texplay"

include Gosu

Win = Window.new(640, 480, false)

context TexPlay, :repeat => 3 do
  context "Image.new", :skip => false do
    before do
      @img = TexPlay.create_image(Win, 100, 100)
    end

    show bench "caching false", :repeat => 5 do
      Image.new(Win, @img, :caching => false)
    end

    bench "caching true" do
      Image.new(Win, @img, :caching => true)
    end
    
    rank "caching false", "caching true"
  end

  context "clear vs filled rec", :repeat => 0, :skip => false do
    before do
      @img = TexPlay.create_image(Win, 500, 500)
    end
    
    show bench "clear" do
      @img.clear :color => :red
    end

    show bench "filled rect" do
      @img.rect 0, 0, @img.width - 1, @img.height - 1, :color => :red, :fill => true
    end

#    rank "clear", "filled rect"
  end
      
  context "TexPlay.create_image", :skip => true do
    bench "with clear" do
      TexPlay.create_image(Win, 500, 500).clear
    end

    bench "without clear" do
      TexPlay.create_image(Win, 500, 500)
    end

    compare "with clear", "without clear"
  end
end

