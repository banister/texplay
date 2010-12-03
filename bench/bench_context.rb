require 'benchmark'

module Baseline
  TIME_MODE_DEFAULT = :total
  
  @time_mode = TIME_MODE_DEFAULT
  
  class << self
    attr_accessor :time_mode
  end
  
  class BenchContext
    attr_reader :total_time
    
    def initialize(repeat, indent_level, before, after)
      @repeat = repeat
      @total_time = 0
      @indent_level = indent_level
      @before = Array(before)
      @after =  Array(after)
      @results = {}
    end

    def before(&block)
      @before.push(block)
    end

    def after(&block)
      @after.unshift(block)
    end

    def exec_hooks(hooks)
      hooks.each do |b|
        instance_eval(&b) 
      end 
    end

    def wrap_with_hooks(options={}, &block)
      exec_hooks(options[:before])
      yield
      exec_hooks(options[:after])
    end

    def time_mode
      Module.nesting[1].time_mode
    end
    
    def bench(name, options={},  &block)
      repeat = options[:repeat] || @repeat
      repeat_text = options[:repeat] ? "(repeat: #{repeat})" : ""
      
      bm_block = proc { repeat.times { yield } }

      time = 0
      wrap_with_hooks(:before => @before, :after => @after) do
        time = Benchmark.measure(&bm_block).send(time_mode)
        puts "#{indenter}#{name}: %0.2f #{repeat_text}" % time
        @total_time += time
      end

      @results[name] = time

      time
    end

    def indenter
      " " * @indent_level
    end

    def context(name, options={}, &block)
      repeat = options[:repeat] || @repeat
      repeat_text = options[:repeat] ? "(repeat: #{repeat})" : ""
      
      puts "#{indenter}--"
      puts "#{indenter}Benching #{name}: #{repeat_text}"

      @total_time +=
        time = BenchContext.new(repeat, @indent_level + 1, @before, @after).
        tap { |v| v.instance_eval(&block) }.
        total_time
      
      puts "#{indenter}total time: %0.2f seconds for #{name}" % time
      puts "#{indenter}--"

      @results[name] = time

      @total_time
    end

    def rank(*names)
      ranking = names.sort_by! { |v| @results[v] }
      quoted_ranking = names.map.with_index { |v, i| "#{i + 1}. \"#{v}\"" }.join(", ")
      puts "#{indenter}Rankings: #{quoted_ranking}"
    end
  end

  module ObjectExtensions
    private
    def context(name, options={}, &block)
      repeat = options[:repeat] || 1
      time_mode = Baseline.time_mode
      time_mode_text = time_mode != Baseline::TIME_MODE_DEFAULT ? "(time mode: #{time_mode})" : ""

      puts "--"
      puts "Benching #{name}: (repeat: #{repeat}) #{time_mode_text}"
      
      top_level_context_time = Baseline::BenchContext.new(repeat, 1, nil, nil).
        tap { |v| v.instance_eval(&block) }.
        total_time

      puts "total time: %0.2f seconds for #{name}" % top_level_context_time
      puts "--"

      top_level_context_time
    end
  end
end

class Object
  include Baseline::ObjectExtensions
end
