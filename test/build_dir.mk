
all: $(SUB_DIRS)

.PHONY:$(SUB_DIRS)
$(SUB_DIRS):
	cd $@ && make
