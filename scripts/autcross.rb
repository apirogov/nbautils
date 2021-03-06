#!/usr/bin/env ruby
require 'open3'

SCRIPTPATH=`echo -n $( cd "$(dirname "$0")" ; pwd -P )`
CWD=`realpath .`.strip
SEED=`date +%s`.strip
DATE=`date +%Y%m%d-%H%M%S`.strip

#----

#argument set combinators, e.g.: compile oneof("a","b").prod prefs("c","d","e")

def oneof(*a); a.map(&->(x){[x]}); end

def prefs(*a); 0.upto(a.length).collect(&->(i){a[0...i]}); end

#list monad
class Array; def prod(rhs); self.product(rhs).map!(&:flatten); end; end
class Array; def app(rhs); self.map!(& lambda { |el| el.push(rhs) }); end; end

def compile(a); a.map(&->(l){l.map(&->(x){x}).join(" ")}); end

#----

def wrap(str); 'cat %H | ' + str + ' > %O'; end

def translator_args(tarr); tarr.map(&->(s){"-t '#{wrap s}'"}).join(' '); end

# ----
# compact representation of different argument combinations
# -c does not work with -o, -c,-p always bad
nbadet_argsets = compile(
  oneof("-u0","-u1","-u2").app('-k -j').prod(prefs('-t', '-i', '-r', '-m', '-o', '-q', '-n -a -b', '-d -e'))
)

translators = ["autfilt -D -P --high"] #default and always present
# translators.push("stupid.sh") #to test failure

# add different nbadet configs
nbadet_argsets.each do |s|
  translators.push("#{SCRIPTPATH}/../build/bin/nbadet #{s}")
end

output_args = "--save-bogus=failed_#{DATE}.hoa --csv=stats_#{DATE}.csv"
cmd = "./autcrossw.rb #{translator_args translators} #{output_args} #{ARGV.join(' ')}"

# ----
# call autcross, pass through IO
Open3.popen3(cmd) do |stdin, stdout, stderr, wait_thr|
  # consume outputs in STDOUT and STDERR, otherwise the process
  # may be blocked if it produces a lot of outputs
  Thread.new do
    stdout.each_line do |l|
      puts l
    end
  end
  Thread.new do
    stderr.each_line do |l|
      STDERR.puts l
    end
  end

  while data = STDIN.read(256) # read output of the upstream command
    stdin.write(data)          # manually pipe it to the ffmpeg command
  end
  stdin.close
  wait_thr.join
end
