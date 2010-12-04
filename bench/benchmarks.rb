direc = File.dirname(__FILE__)

require './bench_context'
require "#{direc}/../lib/texplay"

include Gosu

Baseline.time_mode = :real

Win = Window.new(640, 480, false)

context TexPlay, :repeat => 20 do

  before do
    @img = TexPlay.create_image(Win, 100, 100)
  end
  
  # context "TexPlay#create_image" do
  #   before do
  #   end
  context "create_image" do

    show bench "caching false" do
      Image.new(Win, @img, :caching => false)
    end
  end

  show bench "caching true" do
    Image.new(Win, @img, :caching => true)
  end

  rank "create_image", "caching true"
  compare "create_image", "caching true"
  #  compare :caching_true, :caching_false
  # end

  # show bench :caching_true_outer do
  #   Image.new(@win, @img, :caching => true)
  # end
end

