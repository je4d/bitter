DIR_ := $(DIR)
DIR := bitter
DIR_$(DIR) := $(DIR_)
ifeq ($(JE4D_ROOT),1)

$(eval $(call subdir_start,$(DIR)))

################################################################################

includes := external/bandit

apps := bit                                                                    \
        bit_iterator                                                           \
        bit_iterator_copy_1                                                    \
        bit_iterator_copy_2                                                    \
        bit_iterator_copy_3                                                    \
        bit_iterator_copy_4                                                    \
        offset

################################################################################

$(eval $(call subdir_end,$(DIR)))
$(eval $(call target_add,$(DIR),all,apps))

$(DIR)/all: $(DIR)/test

$(DIR)/test: %/test: $(apps_tgt)
	build/$*/bit --reporter=spec
	build/$*/bit_iterator --reporter=spec
	build/$*/bit_iterator_copy_1 --reporter=spec
	build/$*/bit_iterator_copy_2 --reporter=spec
	build/$*/bit_iterator_copy_3 --reporter=spec
	build/$*/bit_iterator_copy_4 --reporter=spec
	build/$*/offset --reporter=spec

DIR := $(DIR_$(DIR))
else
include ../wrapper.mf
endif
