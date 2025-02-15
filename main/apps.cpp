
#include <stdio.h>
#include "esp_brookesia.hpp"

void install_provisioner_app(ESP_Brookesia_Phone *phone);
void install_nodes_app(ESP_Brookesia_Phone *phone);
void install_on_off_control_app(ESP_Brookesia_Phone *phone);
void install_level_control_app(ESP_Brookesia_Phone *phone);
void install_hsl_control_app(ESP_Brookesia_Phone *phone);

void install_apps(ESP_Brookesia_Phone *phone)
{
	install_provisioner_app(phone);
	install_nodes_app(phone);
	install_on_off_control_app(phone);
	install_level_control_app(phone);
	install_hsl_control_app(phone);
}
