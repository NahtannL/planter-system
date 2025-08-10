idf_component_register(SRCS "rest_api.c" "main.c" "planter_utils.c" "sensor.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "cert/certificate.pem")