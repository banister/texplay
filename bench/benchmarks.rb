direc = File.dirname(__FILE__)

require './bench_context'
require "#{direc}/../lib/texplay"

include Gosu

context TexPlay, :repeat => 0 do

  before do
      @win = Window.new(640, 480, false)
  end
  
  context "TexPlay#create_image", :repeat => 2 do
    before do
      @img = TexPlay.create_image(@win, 100, 100)
    end

    show bench :caching_false do
      Image.new(@win, @img, :caching => false)
    end

    show bench :caching_true do
      Image.new(@win, @img, :caching => true)
    end

    rank :caching_true, :caching_false
    compare :caching_true, :caching_false
  end

  bench "caching: :lazy" do
      Image.new(@win, @img, :caching => :lazy)
  end
end

