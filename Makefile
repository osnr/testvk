testvk: testvk.c
	cc -I./vendor -o $@ $^

remote:
	scp -r * folk-convivial:~/testvk
	ssh folk-convivial -- 'cd testvk; make; ./testvk'
