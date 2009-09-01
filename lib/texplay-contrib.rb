begin
    require 'rubygems'
rescue LoadError
end

require 'texplay'

# setup will be executed straight after Gosu::Image instantiation
TexPlay.on_setup do
    @turtle_pos = TexPlay::TPPoint.new(width / 2, height / 2 )
    @turtle_angle = 0
end

TexPlay.create_macro(:move_to) do |x, y|
    capture { 
        @turtle_pos.x = x
        @turtle_pos.y = y
    }
end

TexPlay.create_macro(:move_rel) do |dx, dy|
    capture { 
        @turtle_pos.x += dx
        @turtle_pos.y += dy
    }
end

TexPlay.create_macro(:line_to) do |x, y, *other|
    capture { 
        line(@turtle_pos.x, @turtle_pos.y, x, y, *other)

        @turtle_pos.x, @turtle_pos.y = x, y
    }
end

TexPlay.create_macro(:line_rel) do |dx, dy, *other|
    capture { 
        x = @turtle_pos.x + dx
        y = @turtle_pos.y + dy

        line(@turtle_pos.x, @turtle_pos.y, x, y, *other)

        @turtle_pos.x, @turtle_pos.y = x, y
    }
end

TexPlay.create_macro(:turn_to) do |a|
    capture {
        @turtle_angle = a
    }
end

TexPlay.create_macro(:turn) do |da|
    capture { 
        @turtle_angle += da
    }
end

TexPlay.create_macro(:forward) do |dist, *other|
    capture {
        visible = other.shift

        radians_per_degree = 0.0174532925199433

        x = @turtle_pos.x + dist * Math::cos(radians_per_degree * @turtle_angle)
        y = @turtle_pos.y + dist * Math::sin(radians_per_degree * @turtle_angle)

        if(visible) then
            line_to(x, y, *other)
        else
            move_to(x, y)
        end
    }
end



