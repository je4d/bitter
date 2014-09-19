DIR_ := $(DIR)
DIR := bitter
DIR_$(DIR) := $(DIR_)
ifeq ($(JE4D_ROOT),1)

$(eval $(call subdir_start,$(DIR)))

################################################################################

#apps := bit

################################################################################

$(eval $(call subdir_end,$(DIR)))
$(eval $(call target_add,$(DIR),all,apps))

#$(DIR)/all: $(DIR)/test

#$(DIR)/test: %/test: $(apps_tgt)
#	build/$*/bit

DIR := $(DIR_$(DIR))
else
include ../wrapper.mf
endif
