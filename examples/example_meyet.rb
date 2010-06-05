require 'rubygems'
require 'common'
require 'gosu'
require 'texplay'



# TEST_CASE 1 shows 2 pieces of text, but the background is black, even with :chroma_key. TEST_CASE 2 shows nothing.
# However, if you remove the :chroma_key's, you see a black background (and no text) in TEST_CASE 2, so there, :chroma_key appears to work.
TEST_CASE = 1 # 1 shows everything, 2 shows nothing. Both, however, should show everything, and haev a working :chroma_key => :alpha
SYNC_TEST = false # Setting to true enables a force_sync.. but no matter where that is placed, it doesn't help

class Wnd < Gosu::Window
  def initialize  
    super(500, 200, false)
    self.caption = "Splice Issues"
    
    @chrome = TexPlay::create_blank_image(self, 200, 200)
    @chrome.fill 0, 0, :color => :purple
    @chrome.rect rand(200),rand(200),rand(200),rand(200), :color => :blue
    @chrome.rect rand(200),rand(200),rand(200),rand(200), :color => :blue
    @chrome.rect rand(200),rand(200),rand(200),rand(200), :color => :blue
    @chrome.rect rand(200),rand(200),rand(200),rand(200), :color => :blue
    
    @chrome3 = @chrome.dup if TEST_CASE == 2
    
    @short_text = Gosu::Image.from_text(self, "text", Gosu::default_font_name, 20)
    @long_text = Gosu::Image.from_text(self, "This is a long piece of text..", Gosu::default_font_name, 20)
    
    @chrome2 = @chrome.dup if TEST_CASE == 1
    
    @chrome.splice @short_text, 5, 5, :alpha_blend => true
    @chrome.splice @long_text, 5, 105, :alpha_blend => true
    puts @long_text.get_pixel(2,2)
    
    if TEST_CASE == 1
      @chrome2.splice @short_text, 105, 5, :alpha_blend => true
      @chrome2.splice @long_text, 105, 105, :alpha_blend => true
    elsif TEST_CASE == 2
      @chrome3.splice @short_text, 105, 5, :alpha_blend => true
      @chrome3.splice @long_text, 105, 105, :alpha_blend => true
    end
  end
  
  def draw
    @chrome.draw(0,0,1)
    @chrome2.draw(200,0,1) if TEST_CASE == 1
    @chrome3.draw(400,0,1) if TEST_CASE == 2
  end
  
  def draw_text
    @chrome.splice @short_text, 5, 40, :alpha_blend => true # Chroma key apparently doesn't work either..
    @chrome2.splice @short_text, 5, 40, :alpha_blend => true if TEST_CASE == 1
    @chrome3.splice @short_text, 5, 40, :alpha_blend => true if TEST_CASE == 2
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
    draw_text if id == Gosu::KbSpace
  end
end

Wnd.new.show

