direc = File.dirname(__FILE__)

require 'benchmark'
require "#{direc}/output_format"


module Baseline
  TIME_MODE_DEFAULT = :total
  
  @time_mode = TIME_MODE_DEFAULT
  @output_format = OutputFormat.new
  @repeat_default = 1
  
  class << self
    attr_accessor :time_mode, :output_format, :repeat_default
  end

  class BenchContext
    attr_reader :total_time, :bench_count,
    :subcontext_count, :repeat
    
    def initialize(repeat, nest_level, before, after)
      @repeat = repeat
      @total_time = 0
      @nest_level = nest_level
      @before = Array(before).dup
      @after =  Array(after).dup
      @results = {}
      @bench_count = @subcontext_count = 0
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

    def output_format
      Module.nesting[1].output_format
    end

    def indenter
      " " * @nest_level * 2
    end

    def exec_bench(name, new_repeat, &block)
      repeat = new_repeat || @repeat
      bm_block = proc { repeat.times { yield } }
      
      time = 0
      wrap_with_hooks(:before => @before, :after => @after) do
        time = Benchmark.measure(&bm_block).send(time_mode)
        @total_time += time
      end

      @results[name] = time
      @bench_count += 1
      
      [name, time, new_repeat, :with_own_block]
    end

    def show(bench_data, &block)
      case bench_data.last
      when :without_own_block
        name, new_repeat  = bench_data
        show exec_bench(name, new_repeat, &block)
      when :with_own_block
        name, time, new_repeat = bench_data
        output_format.bench_output(name, time, new_repeat, @repeat, @nest_level)
      end
    end
    
    def bench(name, options={},  &block)
      
      # if no block then assume block is provided by show method and
      # pass along requisite data so show can do its thing
      if !block_given?
        [name, options[:repeat], :without_own_block]
      else
        exec_bench(name, options[:repeat], &block)
      end
    end

    def context(name, options={}, &block)
      return output_format.context_skip(name) if options[:skip]
      
      repeat = options[:repeat] || @repeat
      
      bc = BenchContext.new(repeat, @nest_level + 1, @before, @after)
      bench_count = subcontext_count = 0
      output_format.context_output_header(name, options[:repeat], @repeat, @nest_level)
      
      @total_time += time = bc.tap do |v|
        v.instance_eval(&block)
        bench_count = v.bench_count
        subcontext_count = v.subcontext_count
      end.
        total_time

      output_format.context_output_footer(name, time, bench_count, subcontext_count, @nest_level)
      @results[name] = time
      @subcontext_count += 1
      @total_time
    end

    def compare(bench1, bench2)
      benches = [bench1, bench2]
      winner, loser = benches.sort_by! { |v| @results[v] }
      time_diff = @results[loser] - @results[winner]
      time_ratio = @results[loser] / @results[winner].to_f

      output_format.compare_output(winner, loser, time_diff, time_ratio, @nest_level)
    end
  end
  
  def rank(*names)
    ranking = names.sort_by! { |v| @results[v] }
    output_format.rank_output(ranking, @indent_level)
  end

  module ObjectExtensions
    private
    def context(name, options={}, &block)
      return output_format.top_level_context_skip(name) if options[:skip]
      
      repeat = options[:repeat] || Baseline.repeat_default
      time_mode = Baseline.time_mode
      output_format = Baseline.output_format

      output_format.top_level_context_output_header(name, options[:repeat], Baseline.repeat_default, 0)
      bench_count = subcontext_count = 0
      
      top_level_context_time = Baseline::BenchContext.new(repeat, 1, nil, nil).
        tap do |v|
        v.instance_eval(&block)
        bench_count = v.bench_count
        subcontext_count = v.subcontext_count
      end.
        total_time

      output_format.context_output_footer(name, top_level_context_time, bench_count, subcontext_count, 0)

      top_level_context_time
    end
  end
end

class Object
  include Baseline::ObjectExtensions
end
