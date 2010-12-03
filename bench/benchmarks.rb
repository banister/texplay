direc = File.dirname(__FILE__)

require './bench_context'
require "#{direc}/../lib/texplay"

include Gosu

context TexPlay, :repeat => 0 do

  context "TexPlay#create_image", :repeat => 10 do
    before do
      @win = Window.new(640, 480, false)
      @img = TexPlay.create_image(@win, 100, 100)
    end

    bench "caching :false", :repeat => 4 do
      Image.new(@win, @img, :caching => false)
    end

    bench "caching: true" do
      Image.new(@win, @img, :caching => true)
    end

    rank "caching: true", "caching :false"
  end

  bench "caching: :lazy" do
      Image.new(@win, @img, :caching => :lazy)
    end
end

