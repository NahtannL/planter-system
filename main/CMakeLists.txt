idf_component_register(SRCS "rest_api.c" "main.c" "planter_utils.c" "sensor.c"
                    "solenoid.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "cert/certificate.pem")