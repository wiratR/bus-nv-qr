#include <time.h>

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
/*
// sample test
void testFirebase ()
{
    String path = "/Test/Json";

    String jsonStr = "";

    FirebaseJson json1;

    FirebaseJsonData jsonObj;

    json1.set("Hi/myInt", 200);
    json1.set("Hi/myDouble", 0.0023);
    json1.set("Who/are/[0]", "you");
    json1.set("Who/are/[1]", "they");
    json1.set("Who/is/[0]", "she");
    json1.set("Who/is/[1]", "he");
    json1.set("This/is/[0]", false);
    json1.set("This/is/[1]", true);
    json1.set("This/is/[2]", "my house");
    json1.set("This/is/[3]/my", "world");

    Serial.println("------------------------------------");
    Serial.println("JSON Data");
    json1.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println("------------------------------------");

    Serial.println("------------------------------------");
    Serial.println("Set JSON test...");

    if (Firebase.set(firebaseData, path, json1))
    {
        Serial.println("PASSED");
        Serial.println("PATH: " + firebaseData.dataPath());
        Serial.println("TYPE: " + firebaseData.dataType());
        Serial.print("VALUE: ");
        printResult(firebaseData);
        Serial.println("------------------------------------");
        Serial.println();
    }
    else
    {
        Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
    }

    Serial.println("------------------------------------");
    Serial.println("Get JSON test...");

    if (Firebase.get(firebaseData, path))
    {
        Serial.println("PASSED");
        Serial.println("PATH: " + firebaseData.dataPath());
        Serial.println("TYPE: " + firebaseData.dataType());
        Serial.print("VALUE: ");
        if (firebaseData.dataType() == "json")
        {
            jsonStr = firebaseData.jsonString();
            printResult(firebaseData);
        }

        Serial.println("------------------------------------");
        Serial.println();
    }
    else
    {
        Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
    }

    Serial.println("------------------------------------");
    Serial.println("Try to parse return data and get value..");

    json1.setJsonData(jsonStr);

    json1.get(jsonObj, "This/is/[3]/my");
    Serial.println("This/is/[3]/my: " + jsonObj.stringValue);

    json1.get(jsonObj, "Hi/myDouble");
    Serial.print("Hi/myDouble: ");
    Serial.println(jsonObj.doubleValue, 4);

    Serial.println("------------------------------------");
    Serial.println();
}
*/
// sent response device status to firebase
int sentResponseDeviceStatus(int deviceId)
{
    // path for db 
    //  dv_status
    //    |--item_id
    //        |--device_id  : 'deviceId'
    //        |--datetime   : <yyyy/mm/dd hh:mm:ss>             'parameter is datetime_now'
    //        |--status     : <In-Service>,<Out-Of-Service>     
    //        |--recheck    : <0> default  
    char deviceBuf[4];
    sprintf (deviceBuf,"%4x",deviceId);         // convert to Hex 4 digits Eg. 1 --> 0001 , 10 ---> 000A
    // get datetime is now
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);
    char datetime_now[50];                      //50 chars should be enough
    strftime(datetime_now, sizeof(datetime_now), "%A, %B %d %Y %H:%M:%S", p_tm);
    String status = "In-Service";       // default
    int recheck = 0;                    // default;
    // DEBUG MESSAGE
    Serial.print( "Result : " );
    Serial.print( deviceBuf );          Serial.print( " - " );
    Serial.print( datetime_now );       Serial.print( " - " );
    Serial.print( status );             Serial.print( " - " );
    Serial.print( recheck );            Serial.println("");

    // Set path for RDS on Firebase 
    String path = "/dv_status/evt_bus_" + deviceBuf[0];

    String jsonStr = "";
    FirebaseJson json1;
    FirebaseJsonData jsonObj;

    json1.set("device_id", deviceBuf);
    json1.set("datetime", datetime_now);
    json1.set("status", status);
    json1.set("recheck", recheck );

    // set firebaseDate to 
    if ( !(Firebase.set(firebaseData, path, json1)) )
    {
        Serial.println("set firebase FAILED");
        Serial.println("REASON: " + firebaseData.errorReason());
    }


    delay(1000);

    return 0;
}

// check "/dv_status/{item_id}/recheck/{value}
// if value = 1 need deive resrnt status
int readRequestDevceiStatus(int deviceId)
{
    // Set path for RDS on Firebase
    String path = "/dv_status/evt_bus_" + deviceBuf[0];

    String jsonStr = "";
    FirebaseJson json1;
    FirebaseJsonData jsonObj;

    json1.get(jsonObj, "This/is/[3]/my");
    return 0;
}


// sent payment details 


// reevice payment 
