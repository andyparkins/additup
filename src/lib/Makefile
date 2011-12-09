
DEPS := additup general

build: $(DEPS:=.d)

clean: $(DEPS:=.d.clean)

%.d:
	$(MAKE) -C $*

%.d.clean:
	$(MAKE) -C $* clean

	
