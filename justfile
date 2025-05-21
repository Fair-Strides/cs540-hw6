
build +FLAGS='-O3 -Wall -Wextra -Wconversion -Wsign-conversion -Wformat-security -mindirect-branch=thunk -mfunction-return=thunk -fstack-protector-all -Wstack-protector --param ssp-buffer-size=4 -fstack-clash-protection -pie -fPIE -ftrapv -D_FORTIFY_SOURCE=2':
  @echo 'Building project...'
  g++ -std=c++11 {{FLAGS}} main.cpp -o main.out

debug *STDIN: (build '-g')
  #!/usr/bin/env bash
  set -euxo pipefail
  echo 'Debugging project...'
  infile="$(mktemp)"
  echo {{STDIN}} >"$infile"
  gdb -ex "set args <$infile" main.out 

clean:
  @echo 'Cleaning project...'
  rm --recursive --force main.out EmployeeIndex.dat __pycache__/

run *STDIN: build
  @echo 'Running project...'
  echo {{STDIN}} | ./main.out

test *ARGS: build
  @echo 'Testing project...'
  python3 -m unittest {{ARGS}}

