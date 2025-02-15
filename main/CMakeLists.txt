idf_component_register(SRCS 
                    "main.cpp" 
                    "apps.cpp"
                    # "lvgl/top_menu.cpp"
                    # "lvgl/screen1.cpp"
                    "lvgl/screen2.cpp"
                    # "lvgl/screen3.cpp"
                    # "lvgl/screen4.cpp"
                    # "lvgl/screen5.cpp"
                    "lvgl/modal_element.cpp"

                    "lvgl/models/model_1000.cpp"
                    "lvgl/models/model_1002.cpp"
                    "lvgl/models/model_1307.cpp"

                    "ble-mesh/main.cpp"
                    "apps/provisioner/provisioner_app.cpp"
                    "apps/provisioner/all_nodes_app.cpp"
                    "apps/cli-nodes/on_off_cli_app.cpp"
                    "apps/cli-nodes/hsl_cli_app.cpp"
                    "apps/cli-nodes/level_cli_app.cpp"
                    INCLUDE_DIRS "." "include" "apps/include")
