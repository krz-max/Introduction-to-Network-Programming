
all: robust_udpsrv.cpp robust_udpcli.cpp
	g++ -static -o udpsrv.out robust_udpsrv.cpp
	g++ -static -o udpcli.out robust_udpcli.cpp

submit:
	python3 submit.py ./udpsrv.out ./udpcli.out XGK4DEP4MXQ5MW2W

.PHONY: clean
clean:
	rm *.out