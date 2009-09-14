begin
    require 'rubygems'
rescue LoadError
end

require 'texplay'

# to bring in String#each_char for 1.8
if RUBY_VERSION =~ /1.8/
    require 'jcode'
end

# setup will be executed straight after Gosu::Image instantiation
TexPlay::on_setup do
    @turtle_pos = TexPlay::TPPoint.new(width / 2, height / 2 )
    @turtle_angle = 0
end


TexPlay::create_macro(:move_to) do |x, y|
    capture { 
        @turtle_pos.x = x
        @turtle_pos.y = y
    }
end

TexPlay::create_macro(:move_rel) do |dx, dy|
    capture { 
        @turtle_pos.x += dx
        @turtle_pos.y += dy
    }
end

TexPlay::create_macro(:line_to) do |x, y, *other|
    capture { 
        line(@turtle_pos.x, @turtle_pos.y, x, y, *other)

        @turtle_pos.x, @turtle_pos.y = x, y
    }
end

TexPlay::create_macro(:line_rel) do |dx, dy, *other|
    capture { 
        x = @turtle_pos.x + dx
        y = @turtle_pos.y + dy

        line(@turtle_pos.x, @turtle_pos.y, x, y, *other)

        @turtle_pos.x, @turtle_pos.y = x, y
    }
end

TexPlay::create_macro(:turn_to) do |a|
    capture {
        @turtle_angle = a
    }
end

TexPlay::create_macro(:turn) do |da|
    capture { 
        @turtle_angle += da
    }
end

TexPlay::create_macro(:forward) do |dist, *other|
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

## L-System code

# monkeypatches to TexPlay class
module TexPlay
    class LSystem
        def initialize(&block)
            @rules = {}
            
            instance_eval(&block) if block
        end
        
        def rule(new_rule)
            @rules.merge!(new_rule)
        end

        def atom(new_atom)
            @atom = new_atom
        end

        def angle(new_angle=nil)
            return @angle if !new_angle
            @angle = new_angle
        end

        def produce_string(order)
            order = order[:order]
            string = @atom.dup
            
            order.times do
                i = 0
                while(i < string.length)
                    sub = @rules[string[i, 1]]
                    
                    string[i] = sub if sub
                    
                    i += sub ? sub.length : 1
                end
            end
            
            string
        end
    end
end

# L-System macro
TexPlay::create_macro(:lsystem) do |x, y, system, options|
    capture {
        theta = system.angle
        turtle_stack = []
        move_to(x, y)
        line_length = options[:line_length] || 1
        
        system.produce_string(options).each_char do |v|
                      
            case v
            when "F"
                forward(line_length, true)
            when "+"
                turn(theta)
            when "-"
                turn(-theta)
            when "["
                turtle_stack.push([@turtle_pos.dup, @turtle_angle])
            when "]"
                @turtle_pos, @turtle_angle = turtle_stack.pop
            end
        end
    }
end
