direc = File.dirname(__FILE__)

require './bench_context'
require "#{direc}/../lib/texplay"

include Gosu

context TexPlay, :repeat => 0 do

  before do
      @win = Window.new(640, 480, false)
  end
  
  context "TexPlay#create_image", :repeat => 10 do
    before do
      @img = TexPlay.create_image(@win, 100, 100)
    end

    show bench("caching :false", :repeat => 2) {
      Image.new(@win, @img, :caching => false)
    }

    show bench "caching: true", :repeat => 3 do
      Image.new(@win, @img, :caching => true)
    end

    rank "caching: true", "caching :false"
  end

  bench "caching: :lazy" do
      Image.new(@win, @img, :caching => :lazy)
    end
end

