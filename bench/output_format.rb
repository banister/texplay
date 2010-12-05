module Baseline
  class OutputFormat
    def indenter(nest_level)
      " " * 2 * nest_level
    end
    
    def bench_output(name, time, new_repeat, orig_repeat, nest_level)
      repeat = new_repeat || orig_repeat
      repeat_text = new_repeat ? "(repeat: #{repeat})" : ""
      puts "#{indenter(nest_level)}#{name}: %0.2f seconds #{repeat_text}" % time
    end
    
    def context_output_header(name, new_repeat, orig_repeat,  nest_level)
      repeat = new_repeat || orig_repeat
      repeat_text = new_repeat ? "(repeat: #{repeat})" : ""
      puts "#{indenter(nest_level)}"
      puts "#{indenter(nest_level)}Benching #{name}: #{repeat_text}"
    end
    
    def context_output_footer(name, time, bench_count, subcontext_count, nest_level)
      puts "#{indenter(nest_level)}Total time: %0.2f seconds for #{name} " \
      "[#{bench_count} benches and #{subcontext_count} subcontexts]" % time
      puts "#{indenter(nest_level)}"
    end

    def context_skip(name)
      puts "[Skipping #{name}]"
    end

    def top_level_context_skip(name)
      puts "All Benchmarks disabled."
    end

    def top_level_context_output_header(name, new_repeat, orig_repeat, nest_level)
      repeat = new_repeat || orig_repeat
      time_mode = Baseline.time_mode
      time_mode_text = time_mode != Baseline::TIME_MODE_DEFAULT ? "(time mode: #{time_mode})" : ""

      repeat_text = new_repeat ? "(repeat: #{repeat})" : ""
      puts "#{indenter(nest_level)}"
      puts "#{indenter(nest_level)}Benching #{name}: #{repeat_text} #{time_mode_text}"      
    end

    def rank_output(ranking, nest_level)
      quoted_ranking = ranking.map.with_index { |v, i| "#{i + 1}. \"#{v}\"" }.join(", ")
      puts "#{indenter(nest_level)}Rankings: #{quoted_ranking}"
    end

    def compare_output(winner, loser, time_diff, time_ratio, nest_level)
      if time_diff != 0
        puts "#{indenter(nest_level)}Comparison: \"#{winner}\" is faster than \"#{loser}\"" \
        " by %0.2f seconds (%0.2f times faster)" % [time_diff, time_ratio]
      else
        puts "#{indenter(nest_level)}Comparison: \"#{winner}\" is the same speed as \"#{loser}\"" 
      end
    end
  end
end
