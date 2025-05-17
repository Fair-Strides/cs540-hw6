
build +FLAGS='-O3 -Wall -Wextra -Wconversion -Wsign-conversion -Wformat-security -mindirect-branch=thunk -mfunction-return=thunk -fstack-protector-all -Wstack-protector --param ssp-buffer-size=4 -fstack-clash-protection -pie -fPIE -ftrapv -D_FORTIFY_SOURCE=2':
  @echo 'Building project...'
  g++ -std=c++11 {{FLAGS}} main.cpp -o main.out

debug *ARGS: (build '-g')
  @echo 'Debugging project...'
  gdb --args main.out {{ARGS}}

clean:
  @echo 'Cleaning project...'
  rm --recursive --force main.out EmployeeRelation.dat __pycache__/

run *ARGS: build
  @echo 'Running project...'
  ./main.out {{ARGS}}

test *ARGS: build
  @echo 'Testing project...'
  python3 -m unittest {{ARGS}}

