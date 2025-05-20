/*
ESP32-OTA-Pull - a library for doing "pull" based OTA ("Over The Air") firmware
updates, where the image updates are posted on the web.

MIT License

Copyright (c) 2022-3 Mikal Hart

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <WiFi.h>

class ESP32OTAPull
{
public:
    enum ActionType { DONT_DO_UPDATE=0, UPDATE_BUT_NO_BOOT=1, UPDATE_AND_BOOT=2 };

    // Return codes from CheckForOTAUpdate
    enum ErrorCode { UPDATE_AVAILABLE_FORCE_REQ = -4,UPDATE_AVAILABLE = -3, NO_UPDATE_PROFILE_FOUND = -2, NO_UPDATE_AVAILABLE = -1, UPDATE_OK = 0, HTTP_FAILED = 1, WRITE_ERROR = 2, JSON_PROBLEM = 3, OTA_UPDATE_FAIL = 4 };

private:
    void (*Callback)(int offset, int totallength) = NULL;
    ActionType Action = UPDATE_AND_BOOT;
    String Board = ARDUINO_BOARD;
    String Device = "";
    String Config = "";
    String CVersion = "";
    bool DowngradesAllowed = false;

    int DownloadJson(const char* URL, String& payload)
    {
        HTTPClient http;
        http.begin(URL);

        // Send HTTP GET request
        int httpResponseCode = http.GET();

        if (httpResponseCode == 200)
        {
            payload = http.getString();
        }

        // Free resources
        http.end();
        return httpResponseCode;
    }

    int DoOTAUpdate(const char* URL, ActionType Action)
    {
        HTTPClient http;
        http.begin(URL);

        // Send HTTP GET request
        int httpResponseCode = http.GET();

        if (httpResponseCode == 200)
        {
            int totalLength = http.getSize();

            // this is required to start firmware update process
            if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                return OTA_UPDATE_FAIL;

            // create buffer for read
            uint8_t buff[1280] = { 0 };

            // get tcp stream
            WiFiClient* stream = http.getStreamPtr();

            // read all data from server
            int offset = 0;
            while (http.connected() && offset < totalLength)
            {
                size_t sizeAvail = stream->available();
                if (sizeAvail > 0)
                {
                    size_t bytes_to_read = min(sizeAvail, sizeof(buff));
                    size_t bytes_read = stream->readBytes(buff, bytes_to_read);
                    size_t bytes_written = Update.write(buff, bytes_read);
                    if (bytes_read != bytes_written)
                    {
                        Serial.printf("Unexpected error in OTA: %d %d %d\n", bytes_to_read, bytes_read, bytes_written);
                        break;
                    }
                    offset += bytes_written;
                    if (Callback != NULL)
                        Callback(offset, totalLength);
                }
            }

            if (offset == totalLength)
            {
                Update.end(true);
                delay(1000);

                // Restart ESP32 to see changes
                if (Action == UPDATE_BUT_NO_BOOT)
                    return UPDATE_OK;
                ESP.restart();
            }
            return WRITE_ERROR;
        }

        http.end();
        return httpResponseCode;
    }

public:
    /// @brief Return the version string of the binary, as reported by the JSON
    /// @return The firmware version
    String GetVersion()
    {
        return CVersion;
    }

    /// @brief Override the default "Device" id (MAC Address)
    /// @param device A string identifying the particular device (instance) (typically e.g., a MAC address)
    /// @return The current ESP32OTAPull object for chaining
    ESP32OTAPull &OverrideDevice(const char *device)
    {
        Device = device;
        return *this;
    }

    /// @brief Override the default "Board" value of ARDUINO_BOARD
    /// @param board A string identifying the board (class) being targeted
    /// @return The current ESP32OTAPull object for chaining
    ESP32OTAPull &OverrideBoard(const char *board)
    {
        Board = board;
        return *this;
    }

    /// @brief Specify a configuration string that must match any "Config" in JSON
    /// @param config An arbitrary string showing the current configuration
    /// @return The current ESP32OTAPull object for chaining
    ESP32OTAPull &SetConfig(const char *config)
    {
        Config = config;
        return *this;
    }

    /// @brief Specify whether downgrades (posted version is lower) are allowed
    /// @param allow_downgrades true if downgrades are allowed
    /// @return The current ESP32OTAPull object for chaining
    ESP32OTAPull &AllowDowngrades(bool allow_downgrades)
    {
        DowngradesAllowed = allow_downgrades;
        return *this;
    }

    /// @brief Specify a callback function to monitor update progress
    /// @param callback Pointer to a function that is called repeatedly during update
    /// @return The current ESP32OTAPull object for chaining
    ESP32OTAPull &SetCallback(void (*callback)(int offset, int totallength))
    {
        Callback = callback;
        return *this;
    }

    /// @brief The main entry point for OTA Update
    /// @param JSON_URL The URL for the JSON filter file
    /// @param CurrentVersion The version # of the current (i.e. to be replaced) sketch
    /// @param ActionType The action to be performed.  May be any of DONT_DO_UPDATE, UPDATE_BUT_NO_BOOT, UPDATE_AND_BOOT (default)
    /// @return ErrorCode or HTTP failure code (see enum above)
    int CheckForOTAUpdate(const char* JSON_URL, const char *CurrentVersion, ActionType Action = UPDATE_AND_BOOT)
    {
		Serial.println("CheckForOTAUpdate,Begin");
        CurrentVersion = CurrentVersion == NULL ? "" : CurrentVersion;

        // Downloading OTA Json...
        String Payload;
        int httpResponseCode = DownloadJson(JSON_URL, Payload);
        if (httpResponseCode != 200)
            return httpResponseCode > 0 ? httpResponseCode : HTTP_FAILED;
            
        String PayloadPretty = Payload;
        PayloadPretty.replace("\n","\r\n");
        Serial.println(PayloadPretty);

        // Deserialize the JSON file downloaded from user's site
        DynamicJsonDocument doc(6000);
        DeserializationError deserialization = deserializeJson(doc, Payload.c_str());
        if (deserialization != DeserializationError::Ok)
            return JSON_PROBLEM;
            
        bool foundProfile = false;

        // Step through the configurations looking for a match
        int config_index=0;
        for (auto config : doc["Configurations"].as<JsonArray>())
        {
            String CBoard = config["Board"].isNull() ? "" : (const char *)config["Board"];
            String CDevice = config["Device"].isNull() ? "" : (const char *)config["Device"];
            CVersion = config["Version"].isNull() ? "" : (const char *)config["Version"];
            String CConfig = config["Config"].isNull() ? "" : (const char *)config["Config"];
            String CURL = config["URL"].isNull() ? "" : (const char *)config["URL"];
            String CFORCE = config["FORCE"].isNull() ? "" : (const char *)config["FORCE"];
            
            int device_search_index = CDevice.indexOf(Device); // check if the "Device" is in the comma spaced list (string) of devices!
            
            Serial.printf("OTA,config_index=%d,device_search_index=%d\r\n",config_index,device_search_index);
			Serial.printf("CBoard()=%s\r\nCDevice()=%s\r\nCVersion()=%s\r\nCConfig()=%s\r\nCFORCE=%s\r\nCURL=%s\r\n",CBoard.c_str(),CDevice.c_str(),CVersion.c_str(),CConfig.c_str(),CFORCE.c_str(),CURL.c_str());
            			            
       		if( (!CBoard.equals(Board)) || (!CConfig.equals(Config)) )
       		{
				// board or config don't match
				Serial.printf("OTA,skipping config_index %d\r\n",config_index);
				continue;
			}
			
			foundProfile=true;
			
			if(!CDevice.isEmpty())
			{
				// we have a list of MAC's
				// device matched with MAC in the JSON data (CDevice)
                if (CVersion.equals("EXCLUDE"))
                {
					// no updated needed!
					return NO_UPDATE_AVAILABLE;					
				}

                if (CVersion != String(CurrentVersion))
                {
					// versions do not match
					if(Action == DONT_DO_UPDATE)
					{
						if(CFORCE.equals("UPDATE_NOW"))
						{
							return UPDATE_AVAILABLE_FORCE_REQ;
						}
						else
						{
							return UPDATE_AVAILABLE;
						}
					}
					else
					{
						return DoOTAUpdate(config["URL"], Action);
					}					
				}
				else
				{
					// no updated needed!
					return NO_UPDATE_AVAILABLE;
				}
			}
			else
            {
				// we don't have a MAC - so this is a default version!
				// matched on Board and Config Alone
                if (CVersion != String(CurrentVersion))
                {
					// versions do not match
					if(Action == DONT_DO_UPDATE)
					{
						if(CFORCE.equals("UPDATE_NOW"))
						{
							return UPDATE_AVAILABLE_FORCE_REQ;
						}
						else
						{
							return UPDATE_AVAILABLE;
						}
					}
					else
					{
						return DoOTAUpdate(config["URL"], Action);
					}					
				}
				else
				{
					// no updated needed!
					return NO_UPDATE_AVAILABLE;
				}
            }
			
			config_index++;
        }
        
        
		Serial.println("CheckForOTAUpdate,Done");
		       
        return foundProfile ? NO_UPDATE_AVAILABLE : NO_UPDATE_PROFILE_FOUND;
    }
};

