DIR_ := $(DIR)
DIR := bitter
DIR_$(DIR) := $(DIR_)
ifeq ($(JE4D_ROOT),1)

$(eval $(call subdir_start,$(DIR)))

################################################################################

includes := external/bandit

apps := bit                                                                    \
        bit_iterator                                                           \
        bit_iterator_copy_quick                                                \
        offset

utilities :=                                                                   \
        bit_iterator_copy_1                                                    \
        bit_iterator_copy_2                                                    \
        bit_iterator_copy_3                                                    \
        bit_iterator_copy_4

custom_generated_units :=                                                      \
        bit_iterator_copy_quick_srcs                                           \
        bit_iterator_copy_quick_const_srcs                                     \
        bit_iterator_copy_quick_dests                                          \
        bit_iterator_copy_quick_const_dests

build/$(DIR)/bit_iterator_copy_quick_srcs.cpp: \
        build/%/bit_iterator_copy_quick_srcs.cpp: \
            %/bit_iterator_copy_quick_gen.py | build/%
	$*/bit_iterator_copy_quick_gen.py srcs > "$@" || (rm -f "$@"; false)
build/$(DIR)/bit_iterator_copy_quick_const_srcs.cpp: \
        build/%/bit_iterator_copy_quick_const_srcs.cpp: \
            %/bit_iterator_copy_quick_gen.py | build/%
	$*/bit_iterator_copy_quick_gen.py srcs const > "$@" || (rm -f "$@"; false)
build/$(DIR)/bit_iterator_copy_quick_dests.cpp: \
        build/%/bit_iterator_copy_quick_dests.cpp: \
            %/bit_iterator_copy_quick_gen.py | build/%
	$*/bit_iterator_copy_quick_gen.py dests > "$@" || (rm -f "$@"; false)
build/$(DIR)/bit_iterator_copy_quick_const_dests.cpp: \
        build/%/bit_iterator_copy_quick_const_dests.cpp: \
            %/bit_iterator_copy_quick_gen.py | build/%
	$*/bit_iterator_copy_quick_gen.py dests const > "$@" || (rm -f "$@"; false)

bit_iterator_copy_quick_objects := bit_iterator_copy_quick_srcs                \
                                   bit_iterator_copy_quick_const_srcs          \
                                   bit_iterator_copy_quick_dests               \
                                   bit_iterator_copy_quick_const_dests

includes += bitter

################################################################################

$(eval $(call subdir_end,$(DIR)))
$(eval $(call target_add,$(DIR),all,apps))

$(DIR)/all: $(DIR)/test

$(DIR)/test: %/test: build/%/bit                                               \
                     build/%/bit_iterator                                      \
                     build/%/bit_iterator_copy_quick                           \
                     build/%/offset
	build/$*/bit
	build/$*/bit_iterator
	build/$*/bit_iterator_copy_quick
	build/$*/offset

$(DIR)/full_test: %/full_test: build/%/bit                                     \
                               build/%/bit_iterator                            \
                               build/%/bit_iterator_copy_1                     \
                               build/%/bit_iterator_copy_2                     \
                               build/%/bit_iterator_copy_3                     \
                               build/%/bit_iterator_copy_4                     \
                               build/%/offset
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
