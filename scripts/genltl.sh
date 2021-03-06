#!/bin/sh
# generate nondeterministic Büchi automata
# for determinisation benching
genltl \
  --ms-phi-h=..4 \
  --sejk-f=1..3,1 \
  --kr-n=1 \
  --kr-nlogn=1 \
  --kv-psi=1 \
  --

#or-g might be already minimal by spot? ("clean" growth)

# spot is used with autfilt -D -P

#following are stutter invariant
  # --fxg-or=..5 \
  # --gf-equiv=..6 \
  # --gf-implies=..5 \
  # --gh-r=..4 \
  # --go-theta=..6 \
  # --ms-example=..7 \
  # --ms-phi-r=..3 \
  # --ms-phi-s=..3 \
  # --or-fg=..7 \
  # --or-g=..11 \
  # --r-left=..8 \
  # --sejk-j= \
  # --sejk-k= \
  # --sejk-patterns= \
  # --tv-uu=..6 \

#following yield det. BA already and hence are useless:
  # --and-f=..8 \
  # --and-fg=..5 \ #+ NBA is always 2 states and DPA 3, boring
  # --and-gf=..8 \
  # --ccj-alpha=..7 \ #+ slow NBA gen
  # --ccj-beta=..10 \
  # --ccj-beta-prime=..10 \
  # --gh-q=..6  \
  # --gxf-and=..5 \
  # --or-gf=..5 \
  # --r-right=..8 \
  # --rv-counter=..5 \
  # --rv-counter-carry=..5 \
  # --rv-counter-carry-linear=..5 \
  # --rv-counter-linear=..5 \
  # --tv-f1=..5 \
  # --tv-f2=..5 \
  # --tv-g1=..5 \
  # --tv-g2=..5 \
  # --u-left=..5 \
  # --u-right=..5 \
  # --hkrss-patterns=..55 \ # 43 of 55 det.
  # --dac-patterns=..55 \ # 47 of 55 det.
  # --eh-patterns=..12 \ #6 of 12 det.
  # --p-patterns=..20 \ #17 of 20 det.
  # --sb-patterns=..27 \ #21 of 27 det.
