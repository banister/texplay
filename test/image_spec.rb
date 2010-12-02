direc = File.dirname(__FILE__)

require "#{direc}/../lib/texplay"

class Module
  public :remove_const
end

describe Gosu::Image do
  before do
    @window = Gosu::Window.new(640, 480, false)
    NormalImageSize = [Gosu::MAX_TEXTURE_SIZE - 2, Gosu::MAX_TEXTURE_SIZE - 2]
    TooLargeImageSize = [2000, 2000]
  end

  after do
    Object.remove_const(:NormalImageSize)
    Object.remove_const(:TooLargeImageSize)
  end
  
  describe "Gosu::Image#new (patched by TexPlay)" do
    describe "caching option" do
      it "should not cache image if :caching => false" do
        source_image = TexPlay.create_image(@window, *NormalImageSize)
        source_image.quad_cached?.should == false

        image = Gosu::Image.new(@window, source_image, :caching => false)
        image.width.should == NormalImageSize[0]
        image.height.should == NormalImageSize[1]

        image.quad_cached?.should == false
      end

      it "should cache the image if :caching => true" do
        source_image = TexPlay.create_image(@window, *NormalImageSize)

        image = Gosu::Image.new(@window, source_image, :caching => true)
        image.width.should == NormalImageSize[0]
        image.height.should == NormalImageSize[1]

        image.quad_cached?.should == true
      end
    end
  end
end
