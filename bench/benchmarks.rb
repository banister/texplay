direc = File.dirname(__FILE__)

require './bench_context'
require "#{direc}/../lib/texplay"

include Gosu
window = Window.new(640, 480, false)
img = TexPlay.create_image(window, 100, 100)

context TexPlay, :repeat => 1 do
  
  context "TexPlay#create_image", :repeat => 10 do
    bench "caching :false" do
      Image.new(window, img, :caching => false)
    end

    bench "caching: true" do
      Image.new(window, img, :caching => true)
    end
  end
end

