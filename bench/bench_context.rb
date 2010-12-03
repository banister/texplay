require 'benchmark'

class BenchContext
  attr_reader :total_time
  
  def initialize(repeat, dots, before, after)
    @repeat = repeat
    @total_time = 0
    @dots = dots
    @before = Array(before)
    @after =  Array(after)
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
    
  def bench(name, options={},  &block)
    repeat = options[:repeat] || @repeat
    repeat_text = options[:repeat] ? "(repeat: #{repeat})" : ""
    
    if repeat
      bm_block = proc { repeat.times { yield } }
    else
      bm_block = block
    end

    wrap_with_hooks(:before => @before, :after => @after) do
      time = Benchmark.measure(&bm_block).total
      puts "#{"." * @dots}#{name}: %0.2f #{repeat_text}" % time
      @total_time += time
    end
  end

  def context(name, options={}, &block)
    repeat = options[:repeat] || @repeat
    repeat_text = options[:repeat] ? "(repeat: #{repeat})" : ""
    
    puts "#{"." * @dots}Benching #{name}: #{repeat_text}"
    
    @total_time +=
      context_time = BenchContext.new(repeat, @dots + 1, @before, @after).
      tap { |v| v.instance_eval(&block) }.
      total_time
    
    puts "#{"." * @dots}total time for #{name} context: %0.2f seconds" % context_time
  end
end

def context(name, options={}, &block)
  repeat = options[:repeat] || 1
  puts "Benching #{name}: (repeat: #{repeat})"

  context_time = BenchContext.new(repeat, 1, nil, nil).
    tap { |v| v.instance_eval(&block) }.
    total_time

  puts "total running time for #{name} context was %0.2f seconds" % context_time
end
  
