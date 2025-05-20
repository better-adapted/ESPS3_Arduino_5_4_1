idf_component_register(SRCS 
					"app_main_defines.h"
					"app_main.cpp"
					"App_Common.cpp"
					"esp_info.cpp"					
					"libraries/CLI_Arduino/src/CLI.cpp"
					
					
					INCLUDE_DIRS 
				    "."
				    "libraries/CLI_Arduino/src"
					"libraries/ArduinoJson/src"				    
					"libraries/ESP32-OTA-Pull/src"
)