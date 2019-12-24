// function print result
void printResult(FirebaseData &data)
{
    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string")
        Serial.println(data.stringData());
    else if (data.dataType() == "json")
    {
        Serial.println();
        FirebaseJson &json = data.jsonObject();
        //Print all object data
        Serial.println("Pretty printed JSON data:");
        String jsonStr;
        json.toString(jsonStr, true);
        Serial.println(jsonStr);
        Serial.println();
        Serial.println("Iterate JSON data:");
        Serial.println();
        size_t len = json.iteratorBegin();
        String key, value = "";
        int type = 0;
        for (size_t i = 0; i < len; i++)
        {
            json.iteratorGet(i, type, key, value);
            Serial.print(i);
            Serial.print(", ");
            Serial.print("Type: ");
            Serial.print(type == JSON_OBJECT ? "object" : "array");
            if (type == JSON_OBJECT)
            {
                Serial.print(", Key: ");
                Serial.print(key);
            }
            Serial.print(", Value: ");
            Serial.println(value);
        }
        json.iteratorEnd();
    }
    else if (data.dataType() == "array")
    {
        Serial.println();
        //get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray &arr = data.jsonArray();
        //Print all array values
        Serial.println("Pretty printed Array:");
        String arrStr;
        arr.toString(arrStr, true);
        Serial.println(arrStr);
        Serial.println();
        Serial.println("Iterate array values:");
        Serial.println();
        for (size_t i = 0; i < arr.size(); i++)
        {
            Serial.print(i);
            Serial.print(", Value: ");

            FirebaseJsonData &jsonData = data.jsonData();
            //Get the result data from FirebaseJsonArray object
            arr.get(jsonData, i);
            if (jsonData.typeNum == JSON_BOOL)
                Serial.println(jsonData.boolValue ? "true" : "false");
            else if (jsonData.typeNum == JSON_INT)
                Serial.println(jsonData.intValue);
            else if (jsonData.typeNum == JSON_DOUBLE)
                printf("%.9lf\n", jsonData.doubleValue);
            else if (jsonData.typeNum == JSON_STRING ||
                     jsonData.typeNum == JSON_NULL ||
                     jsonData.typeNum == JSON_OBJECT ||
                     jsonData.typeNum == JSON_ARRAY)
                Serial.println(jsonData.stringValue);
        }
    }
}

// sent response device status to firebase
int sentResponseDeviceStatus(int deviceId, char *IPAddr, int status_mode)
{
    // path for db
    //  dv_status
    //    |--item_id
    //        |--device_id  : 'deviceId'                        'parameter is IPAddr'
    //        |--datetime   : <yyyy/mm/dd hh:mm:ss>             'parameter is datetime_now'
    //        |--status     : <In-Service>,<Out-Of-Service>
    //        |--recheck    : <0> default
    char deviceBuf[5];
    int isNewDevice = 0;
    sprintf(deviceBuf, "%04X", deviceId); // convert to Hex 4 digits Eg. 1 --> 0001 , 10 ---> 000A
    // get datetime is now
    char datetime_buf[sizeof "1980-01-01T12:34:56.789Z"];
    time_t rawtime;
    struct tm *timeInfo;
    time(&rawtime);
    timeInfo = localtime(&rawtime);
    snprintf(datetime_buf, sizeof datetime_buf, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             timeInfo->tm_year + 1900,
             timeInfo->tm_mon + 1,
             timeInfo->tm_mday,
             timeInfo->tm_hour,
             timeInfo->tm_min,
             timeInfo->tm_sec,
             0);

    char item_id[50];
    sprintf(item_id, "evt_bus_%s", deviceBuf);     // set item_id is 'evt_bus_dddd'

    if (!Firebase.beginStream(firebaseData, DV_STATUS))
    {
        Serial.println("------------------------------------");
        Serial.println("Can't begin stream connection...");
        Serial.println("REASON: " + firebaseData.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
        return -1;
    }

    char status_item_id[100];
    sprintf(status_item_id, "%s/%s", DV_STATUS, deviceBuf); // '/dv_satuas/evt_bus_dddd'

    char *temp_status;
    switch (status_mode)
    {
    case 1:
        temp_status = "In-Service";
        break;
    default:
        temp_status = "Out-Of-Service";
        break;
    }

    //Declare FirebaseJson object (global or local)
    FirebaseJson json;

    json.set("/deviceId", String(IPAddr));
    json.set("/datetime", String(datetime_buf));
    json.set("/status",   String(temp_status));
    json.set("/recheck",  "0");

    if (!Firebase.beginStream(firebaseData, status_item_id))
    {
        Serial.println("------------------------------------");
        Serial.println("Can't begin stream connection...");
        Serial.println("REASON: " + firebaseData.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
        isNewDevice = 1;
    }

    if (isNewDevice != 0)
    {
        Serial.println("-------------------------------------");
        Serial.println("Add new device status..");
        Serial.println("-------------------------------------");
        // Insert a new item_id to '/dv_status/item_id/'
        // firebaseData is still /dv_satuas
        if (Firebase.pushJSON(firebaseData, status_item_id, json))
        {
            Serial.println("PASSED");
            Serial.println("PATH: " + firebaseData.dataPath());
            Serial.print("PUSH NAME: ");
            Serial.println(firebaseData.pushName());
            Serial.println("ETag: " + firebaseData.ETag());
            Serial.println("------------------------------------");
            Serial.println();
        }
    }
    else
    {
        Serial.println("-------------------------------------");
        Serial.print("Update device : "); Serial.println(item_id);
        Serial.println("-------------------------------------");
        // update on node '/dv_status/item_id/'
        if (Firebase.updateNode(firebaseData, status_item_id, json))
        {
            Serial.println(firebaseData.dataPath());
            Serial.println(firebaseData.dataType());
            Serial.println(firebaseData.jsonString());
        }
        else
        {
            Serial.println(firebaseData.errorReason());
        }
    }
    
    delay(500);

    return 0;
}

    /*

// check "/dv_status/{item_id}/recheck/{value}
// if value = 1 need deive resrnt status
int readRequestDevceiStatus(int deviceId)
{
    // Set path for RDS on Firebase
    char deviceBuf[4];
    sprintf(deviceBuf, "%4x", deviceId); // convert to Hex 4 digits Eg. 1 --> 0001 , 10 ---> 000A
    String pathRecheck = "/dv_status/evt_bus_" + deviceBuf[0];

    if (!(Firebase.getInt(firebaseData, pathRecheck)))
    {
        Serial.println(firebaseData.errorReason());
        return -1;        
    }
    else
    {
        if (!(firebaseData.dataType() == "int"))
        {
            Serial.println("get recheck value failed is wrong type");
            return -1;
                
        }
        else
        {
            Serial.println(firebaseData.intData());
            return firebaseData.intData();
        }
    }
}
*/
    // sent payment details

    // reevice payment
