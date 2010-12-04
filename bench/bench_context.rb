require 'benchmark'

module Baseline
  TIME_MODE_DEFAULT = :total
  
  @time_mode = TIME_MODE_DEFAULT
  
  class << self
    attr_accessor :time_mode
  end

  module Outputter
    def bench_output_header() "" end
    def bench_outout_footer(name, repeat_text, nest_level)
      puts "#{indenter}#{name}: %0.2f seconds #{repeat_text}." % time
    end
    
    def context_output_header(name, repeat_text, nest_level)
      puts "#{indenter}--"
      puts "#{indenter}Benching #{name}: #{repeat_text}"
    end
    
    def context_output_footer(name, time, repeat_text, bench_count, subcontext_count, nest_level)
      puts "#{indenter}Total time: %0.2f seconds for #{name} " \
      "[#{bench_count} benches and #{subcontext_count} subcontexts]" % time
      puts "#{indenter}--"
    end
  end
  
  class BenchContext
    attr_reader :total_time, :bench_count, :subcontext_count
    
    def initialize(repeat, indent_level, before, after)
      @repeat = repeat
      @total_time = 0
      @indent_level = indent_level
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

    def indenter
      " " * @indent_level
    end

    def exec_bench(name, orig_repeat, &block)
      repeat = orig_repeat || @repeat
      repeat_text = orig_repeat ? "(repeat: #{repeat})" : ""

      bm_block = proc { repeat.times { yield } }
        
      time = 0
      wrap_with_hooks(:before => @before, :after => @after) do
        time = Benchmark.measure(&bm_block).send(time_mode)
        @total_time += time
      end

      @results[name] = time
      @bench_count += 1
      
      [name, time, repeat_text, :with_own_block]
    end

    def show(bench_data, &block)
      case bench_data.last
      when :without_own_block
        name, repeat  = bench_data
        show exec_bench(name, repeat, &block)
      when :with_own_block
        name, time, repeat_text = bench_data
        puts "#{indenter}#{name}: %0.2f seconds #{repeat_text}." % time
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
      repeat = options[:repeat] || @repeat
      repeat_text = options[:repeat] ? "(repeat: #{repeat})" : ""
      
      bc = BenchContext.new(repeat, @indent_level + 1, @before, @after)
      bench_count = subcontext_count = 0
      context_output_header(name, repeat_text)
      
      @total_time += time = bc.tap do |v|
        v.instance_eval(&block)
        bench_count = v.bench_count
        subcontext_count = v.subcontext_count
      end.
        total_time

      context_output_footer(name, time, repeat_text, bench_count, subcontext_count)
      @results[name] = time
      @subcontext_count += 1
      @total_time
    end

    def context_output_header(name, repeat_text)
      puts "#{indenter}--"
      puts "#{indenter}Benching #{name}: #{repeat_text}"
    end

    def context_output_footer(name, time, repeat_text, bench_count, subcontext_count)
      puts "#{indenter}Total time: %0.2f seconds for #{name} " \
      "[#{bench_count} benches and #{subcontext_count} subcontexts]" % time
      puts "#{indenter}--"
    end

    def compare(bench1, bench2)
      benches = [bench1, bench2]
      winner, loser = benches.minmax_by { |v| @results[v] }
      time_diff = @results[loser] - @results[winner]
      time_ratio = @results[loser] / @results[winner].to_f
      puts "#{indenter}Comparison: \"#{winner}\" is faster than \"#{loser}\"" \
      " by %0.2f seconds (%0.2f times faster)" % [time_diff, time_ratio]
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
      
      bench_count = subcontext_count = 0
      
      top_level_context_time = Baseline::BenchContext.new(repeat, 1, nil, nil).
        tap do |v|
          v.instance_eval(&block)
          bench_count = v.bench_count
          subcontext_count = v.subcontext_count
        end.
        total_time

      puts "Total time: %0.2f seconds for #{name} " \
      "[#{bench_count} benches and #{subcontext_count} subcontexts]" % top_level_context_time
      puts "--"

      top_level_context_time
    end
  end
end

class Object
  include Baseline::ObjectExtensions
end
