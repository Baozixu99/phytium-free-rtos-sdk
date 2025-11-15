ifdef CONFIG_USE_LIBMETAL

	CSRCS_RELATIVE_FILES += $(wildcard metal/*.c)
	CSRCS_RELATIVE_FILES += $(wildcard metal/system/generic/ft_platform/*.c)
	CSRCS_RELATIVE_FILES += $(wildcard metal/system/generic/*.c)

endif #CONFIG_USE_LIBMETAL