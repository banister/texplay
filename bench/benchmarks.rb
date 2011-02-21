direc = File.dirname(__FILE__)

require 'rubygems'
require 'baseline'
require "#{direc}/../lib/texplay"

include Gosu

Win = Window.new(640, 480, false)

context TexPlay, :repeat => 1 do
  context "Image.new", :skip => true do
    before do
      @img = TexPlay.create_image(Win, 100, 100)
    end

    show bench "caching false", :repeat => 0 do
      Image.new(Win, @img, :caching => false)
    end

    bench "caching true" do
      Image.new(Win, @img, :caching => true)
    end
    
    rank "caching false", "caching true"
  end

  context "color_control vs each" do
    before do
      @img = TexPlay.create_image(Win, 500, 500)
    end

    show bench "each" do
      @img.each do |c|
        c[0] = 1
        c[1] *= 0.5
        c[2] *= 0.5
        c[3] *= 0.5
      end
    end

    show bench "color_control" do
      @img.clear :color_control => proc { |c|
        c[0] = 1
        c[1] *= 0.5
        c[2] *= 0.5
        c[3] *= 0.5
        c
      }
    end

    compare "color_control", "each"
  end

  context "clear vs filled rec", :repeat => 0, :skip => true do
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

  context "get_pixel", :repeat => 20 do
    before do
      @img = TexPlay.create_image(Win, 100, 100).clear :color => :red
    end

    show bench "normal color mode" do
      (0...@img.width).each do |x|
        (0...@img.height).each do |y|
          @img.get_pixel x, y
        end
      end
    end

    show bench "img.each" do
#      @img.each { |v| puts v.inspect }
    end
    
    show bench "gosu color mode" do
      (0...@img.width).each do |x|
        (0...@img.height).each do |y|
          @img.get_pixel x, y, :color_mode => :gosu
        end
      end
    end

    rank "normal color mode", "gosu color mode", "img.each"
    compare "normal color mode", "gosu color mode"

  end
end

