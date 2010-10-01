$LOAD_PATH.unshift File.dirname(File.expand_path(__FILE__))
require 'common'



# TEST_CASE 1 shows 2 pieces of text, but the background is black, even with :chroma_key. TEST_CASE 2 shows nothing.
# However, if you remove the :chroma_key's, you see a black background (and no text) in TEST_CASE 2, so there, :chroma_key appears to work.

class Wnd < Gosu::Window
  def initialize  
    super(500, 200, false)
    self.caption = "Splice Issues"
    
    @chrome = TexPlay::create_blank_image(self, 200, 200)
    @sunset = Gosu::Image.new(self, "#{Common::MEDIA}/sand1.png")

    
    @long_text = Gosu::Image.from_text(self, "This is a long piece of text..", Gosu::default_font_name, 60)
    @chrome.splice @long_text, 0, 0

    @chrome.rect 0,0, @chrome.width, @chrome.height, :texture => @sunset, :fill => true, :mode => :multiply
    
  end
  
  def draw
    @chrome.draw(0,0,1)
  end
end

Wnd.new.show

