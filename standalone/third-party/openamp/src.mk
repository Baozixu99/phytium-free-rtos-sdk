ifdef CONFIG_USE_OPENAMP


CSRCS_RELATIVE_FILES += $(wildcard lib/*.c \
						lib/remoteproc/*.c \
						lib/rpmsg/*.c \
						lib/service/rpmsg/rpc/*.c \
						lib/utils/*.c \
						lib/virtio/*.c \
						ports/*.c \
						openamp-system-reference/examples/legacy_apps/machine/phytium/*.c \
						openamp-system-reference/examples/legacy_apps/system/generic/machine/phytium/*.c )
			 

endif #CONFIG_USE_OPENAMP