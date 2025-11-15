
CSRCS_RELATIVE_FILES += ComputeLibrary/Source/arm_cl_tables.c

ifdef CONFIG_CMSIS_DSP_BASIC_MATH_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/BasicMathFunctions/BasicMathFunctions.c
	CSRCS_RELATIVE_FILES += Source/BasicMathFunctions/BasicMathFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_BAYES_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/BayesFunctions/BayesFunctions.c
	CSRCS_RELATIVE_FILES += Source/BayesFunctions/BayesFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_COMMON_TABLES
	CSRCS_RELATIVE_FILES += Source/CommonTables/CommonTables.c
	CSRCS_RELATIVE_FILES += Source/CommonTables/CommonTablesF16.c
endif

ifdef CONFIG_CMSIS_DSP_COMPLEX_MATH_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/ComplexMathFunctions/ComplexMathFunctions.c
	CSRCS_RELATIVE_FILES += Source/ComplexMathFunctions/ComplexMathFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_CONTROLLER_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/ControllerFunctions/ControllerFunctions.c
endif

ifdef CONFIG_CMSIS_DSP_DISTANCE_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/DistanceFunctions/DistanceFunctions.c
	CSRCS_RELATIVE_FILES += Source/DistanceFunctions/DistanceFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_FAST_MATH_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/FastMathFunctions/FastMathFunctions.c
	CSRCS_RELATIVE_FILES += Source/FastMathFunctions/FastMathFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_FILTERING_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/FilteringFunctions/FilteringFunctions.c
	CSRCS_RELATIVE_FILES += Source/FilteringFunctions/FilteringFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_INTERPOLATION_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/InterpolationFunctions/InterpolationFunctions.c
	CSRCS_RELATIVE_FILES += Source/InterpolationFunctions/InterpolationFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_MATRIX_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/MatrixFunctions/MatrixFunctions.c
	CSRCS_RELATIVE_FILES += Source/MatrixFunctions/MatrixFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_QUATERNION_MATH_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/QuaternionMathFunctions/QuaternionMathFunctions.c
endif

ifdef CONFIG_CMSIS_DSP_STATISTICS_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/StatisticsFunctions/StatisticsFunctions.c
	CSRCS_RELATIVE_FILES += Source/StatisticsFunctions/StatisticsFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_SUPPORT_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/SupportFunctions/SupportFunctions.c
	CSRCS_RELATIVE_FILES += Source/SupportFunctions/SupportFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_SVM_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/SVMFunctions/SVMFunctions.c
	CSRCS_RELATIVE_FILES += Source/SVMFunctions/SVMFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_TRANSFORM_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/TransformFunctions/TransformFunctions.c
	CSRCS_RELATIVE_FILES += Source/TransformFunctions/TransformFunctionsF16.c
endif

ifdef CONFIG_CMSIS_DSP_WINDOW_FUNCTIONS
	CSRCS_RELATIVE_FILES += Source/WindowFunctions/WindowFunctions.c
endif
