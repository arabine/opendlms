
LOCAL_DIR = $(call my-dir)/

# test_aes128gcm.c test_csm_array.c

SOURCES += $(addprefix $(LOCAL_DIR), main.cpp test_clock.cpp test_association.cpp test_csm_array.cpp test_cosem_read_by_block.cpp test_hdlc.cpp cosem_tests_hal.c)

