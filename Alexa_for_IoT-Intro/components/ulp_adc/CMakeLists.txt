if (CONFIG_ULP_COPROC_ENABLED)

    set(COMPONENT_ADD_INCLUDEDIRS include)
    # Edit following two lines to set component requirements (see docs)
    set(COMPONENT_SRCS ./ulp_adc.c)
endif()

set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES ulp )
register_component()

if (CONFIG_ULP_COPROC_ENABLED)
    set(ulp_app_name ulp_adc_button)
    set(ulp_s_sources "ulp/adc_button.S")
    set(ulp_exp_dep_srcs "ulp_adc.c")
    ulp_embed_binary(${ulp_app_name} ${ulp_s_sources} ${ulp_exp_dep_srcs})

endif()