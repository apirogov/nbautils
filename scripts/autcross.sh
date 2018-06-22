#!/bin/bash
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
SEED=$(date +%s)

wrap() {
  echo 'cat %H |' $@ '> %O'
}

# randaut --seed=1234 -u -B -n 500 1..3 -Q 4..10 |
# cat <(echo -e '0\n1') <(randltl --seed $SEED --tree-size 15..20 4 -n 1000 -r3) |
#   ltlfilt -u --remove-wm --size 40..70 | head -n 300 |
#   awk '{ print length($0) " " $0; }' $file | sort -n | cut -d ' ' -f 2- |
#   ltl2tgba -B |
cat |
 autcross -T 180 \
  -t "$(wrap autfilt -D -C -P)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -k -j -t -n -u0)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -k -j -t -n -a -u0)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -k -j -t -n -a -b -u0)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -k -j -t -n -a -b -c -u0)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -k -j -t -n -a -b -c -e -u0)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -k -j -t -n -u1)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -k -j -t -n -a -u1)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -k -j -t -n -a -b -u1)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -k -j -t -n -a -b -c -u1)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -k -j -t -n -a -b -c -e -u1)" \
  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -n -a -c -u0)" \
 $@
  # -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -d )" \
  # -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -d -s)" \
  # -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -d -s -n -a)" \
  # -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -d -s -n -a -c)" \
  # -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -d -s -n -a -c -t)" \
  # -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -d -s -n -a -c -t -g)" \
  # -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -d -s -n -a -c -t -g -p -m)" \
  # -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -d -s -n -a -c -t -g -p -m -u1)" \
  # -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -d -s -n -a -c -t -g -p -m -u1 -l)" \
#  -t "$(wrap $SCRIPTPATH/../build/bin/nbadet -d -s -n -a -c -t -g -u1 -l -p -m)" \
  # -t "$(wrap ltl2dstar -B -H - -)" \
