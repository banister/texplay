$LOAD_PATH.unshift File.join(File.expand_path(__FILE__), '..', 'lib','texplay')
direc = File.dirname(__FILE__)

require "#{direc}/../lib/texplay"

describe TexPlay do
  described_class = TexPlay
  
  before do
    @window = Gosu::Window.new(640, 480, false)
  end

  describe "#create_image" do
    it "should create a blank image of the correct size" do
      width, height = 30, 10
      image = described_class.create_image(@window, width, height)

      image.width.should == width
      image.height.should == height

      width.times do |x|
        height.times do |y|
          image.get_pixel(x, y).should == [0.0, 0.0, 0.0, 0.0]
        end
      end
    end

    it "should create a coloured image of the correct size" do
      width, height = 10, 30
      color = [1.0, 1.0, 0.0, 1.0]
      image = described_class.create_image(@window, width, height, :color => color)

      image.width.should == width
      image.height.should == height

      width.times do |x|
        height.times do |y|
          image.get_pixel(x, y).should == color
        end
      end
    end

    it "should raise an error if an image dimension is 0 or less" do
      lambda { described_class.create_image(@window, 0, 0)}.should raise_error(ArgumentError)
    end

    # TODO: Should probably be an ArgumentError.
    it "should raise an ArgumentError if the image would be too large" do
      too_big = TexPlay::TP_MAX_QUAD_SIZE + 1
      [[too_big, 5], [10, too_big], [too_big, too_big]].each do |width, height|
        puts "WIDTH: #{width.inspect}, HEIGHT: #{height.inspect}"
        lambda {
          described_class.create_image(@window, width, height)
        }.should raise_error(ArgumentError)
      end
    end
  end

  describe "#clear" do
    it "should clear an image to the correct color" do
      img = described_class.create_image(@window, 10, 10)
      img.clear :color => :red
      (0...img.width).each do |x|
        (0...img.height).each do |y|
          img.get_pixel(x,y).should == [1,0,0,1]
        end
      end
    end
  end

  describe "#from_blob" do
    # it "should create an image with the requested pixel data and size" do
    #   # 4 x 3, with columns of red, blue, green, transparent.
    #   gosu_colors = [[255, 0, 0, 255], [0, 255, 0, 255], [0, 0, 255, 255], [0, 0, 0, 0]]
    #   texplay_colors = gosu_colors.map {|a| a.map {|c| c / 255.0 } }
    #   width, height = gosu_colors.size, 3
      
    #   image = described_class.from_blob(@window, (gosu_colors * height).flatten.pack('C*'), width, height)

    #   image.width.should == width
    #   image.height.should == height

    #   texplay_colors.each_with_index do |color, x|
    #     3.times do |y|
    #       image.get_pixel(x, y).should == color
    #     end
    #   end
    # end

    it "should raise an error if the image size is not correct for the blob data" do
      lambda { described_class.from_blob(@window, [1, 1, 1, 1].pack("C*"), 2, 1) }.should raise_error(ArgumentError)
    end

    it "should raise an error if an image dimension is 0 or less" do
      lambda { described_class.from_blob(@window, '', 0, 0) }.should raise_error(ArgumentError)
    end
  end

  describe "alpha_blend" do
    it "should alpha blend using source alpha and not fixing alpha" do
      img1 = described_class.create_image(@window, 10, 10).clear :color => [1, 0, 0, 1]
      img1.clear :color => [0, 0, 1, 0.5], :alpha_blend => :source
      img1.get_pixel(5, 5).should == [0.5, 0, 0.5, 0.75]
    end

    it "hash params true and :source should be equivalent" do
      img1 = described_class.create_image(@window, 10, 10).clear :color => [1, 0, 0, 1]
      img2 = img1.dup
      img1.clear :color => [0, 0, 1, 0.5], :alpha_blend => :source
      img2.clear :color => [0, 0, 1, 0.5], :alpha_blend => true
      
      img1.get_pixel(5, 5).should == [0.5, 0, 0.5, 0.75]
      img2.get_pixel(5, 5).should == [0.5, 0, 0.5, 0.75]
    end

    it "false or nil should prevent alpha blending" do
      img1 = described_class.create_image(@window, 10, 10).clear :color => [1, 0, 0, 1]
      img2 = img1.dup
      img1.clear :color => [0, 0, 1, 0.5], :alpha_blend => false
      img2.clear :color => [0, 0, 1, 0.5], :alpha_blend => nil
      
      img1.get_pixel(5, 5).should == [0, 0, 1, 0.5]
      img2.get_pixel(5, 5).should == [0, 0, 1, 0.5]
    end

    it "should alpha blend using dest alpha and not fixing alpha" do
      img1 = described_class.create_image(@window, 10, 10).clear :color => [1, 0, 0, 0.5]
      img1.clear :color => [0, 0, 1, 1], :alpha_blend => :dest
      img1.get_pixel(5, 5).should == [0.5, 0, 0.5, 0.75]
    end

    it "should alpha blend using source alpha and fixing alpha" do
      img1 = described_class.create_image(@window, 10, 10).clear :color => [1, 0, 0, 1]
      img1.clear :color => [0, 0, 1, 0.5], :alpha_blend => :source_with_fixed_alpha
      img1.get_pixel(5, 5).should == [0.5, 0, 0.5, 1]
    end

    it "should alpha blend using dest alpha and fixing alpha" do
      img1 = described_class.create_image(@window, 10, 10).clear :color => [1, 0, 0, 0.5]
      img1.clear :color => [0, 0, 1, 1], :alpha_blend => :dest_with_fixed_alpha
      img1.get_pixel(5, 5).should == [0.5, 0, 0.5, 0.5]
    end
  end
end
