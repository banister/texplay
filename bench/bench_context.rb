require 'benchmark'

class BenchContext
  attr_reader :total_time
  
  def initialize(repeat, dots)
    @repeat = repeat
    @total_time = 0
    @dots = dots
  end

  def before(&block)
    @before = block
  end

  def after(&block)
    @after = block
  end
  
  def bench(name, options={},  &block)
    repeat = options[:repeat] || @repeat
    repeat_text = options[:repeat] ? "(repeat: #{repeat})" : ""
    
    if repeat
      bm_block = proc { repeat.times { yield } }
    else
      bm_block = block
    end

    @before.call if @before
    time = Benchmark.measure(&bm_block).total
    print_dots
    puts "#{name}: %0.2f #{repeat_text}" % time
    @total_time += time
    @after.call if @after
  end

  def print_dots
    print "." * @dots
  end

  def context(name, options={}, &block)
    repeat = options[:repeat] || @repeat
    repeat_text = options[:repeat] ? "(repeat: #{repeat})" : ""
    
    print_dots
    puts "Benching #{name}: #{repeat_text}"
    
    @total_time +=
      context_time = BenchContext.new(repeat, @dots + 1).
      tap { |v| v.instance_eval(&block) }.
      total_time
    
    print_dots
    puts "total time for #{name} context: %0.2f seconds" % context_time
  end
end

def context(name, options={}, &block)
  repeat = options[:repeat] || 0
  puts "Benching #{name}: (repeat: #{repeat})"

  context_time = BenchContext.new(options[:repeat], 1).
    tap { |v| v.instance_eval(&block) }.
    total_time

  puts "total running time for #{name} context was %0.2f seconds" % context_time
end
  
