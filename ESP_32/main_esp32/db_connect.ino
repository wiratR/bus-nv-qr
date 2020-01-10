

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
int sentResponseDeviceStatus(int status_mode )
{
    if (!Firebase.beginStream(firebaseData1, path_dv_staus))
    {
        Serial.println("-------------------------------------------------------------");
        Serial.println("sentResponseDeviceStatus() : Can't begin stream connection...");
        Serial.println("REASON: " + firebaseData1.errorReason());
        Serial.println("--------------------------------------------------------------");
        Serial.println();
        return -1;
    }

    delay(100);

    if (!Firebase.readStream(firebaseData1))
    {
        Serial.println("--------------------------------------------------------");
        Serial.println("sentResponseDeviceStatus() : Can't read stream data");
        Serial.println("REASON: " + firebaseData1.errorReason());
        Serial.println("-------------------------------------------------------");
        Serial.println();
        return -1;
    }

    if (firebaseData1.streamTimeout())
    {
        Serial.println("Stream timeout, resume streaming...");
        Serial.println();
    }

    FirebaseJson json_dv_staus;
    // add node for update device status
    // path for db
    //  dv_status
    //    |--item_id
    //        |--device_id  : 'deviceId'                        'parameter is IPAddr'
    //        |--datetime   : <yyyy/mm/dd hh:mm:ss>             'parameter is datetime_now'
    //        |--status     : <In-Service>,<Out-Of-Service>
    //        |--recheck    : <0> default
    //        |--location
    //              |--latitude   :
    //              |--longitude  :
    //              |--accuracy   :


    String strMode = "";

    if(status_mode == 1)
        strMode = "In-Service";
    if(status_mode == 2)
        strMode = "Out-Off-Service";

    json_dv_staus.add("device_id",  String(device_ip))
        .add("datetime",            String(formattedDate))
        .add("status",              strMode)
        .add("recheck",             "0")
        .add("/location/latitude",  String(dv_location.lat, 7))
        .add("/location/longitude", String(dv_location.lon, 7))
        .add("/location/accuracy",  String(dv_location.accuracy));

    if (Firebase.updateNode(firebaseData1, path_dv_staus, json_dv_staus))
    {
        if (more_text)
        {
            Serial.println("--------------------------------------------------------");
            Serial.println("sentResponseDeviceStatus() : updatenode with details");
            Serial.println("PATH: " + firebaseData1.dataPath());
            Serial.println("TYPE: " + firebaseData1.dataType());
            Serial.print("VALUE: ");
            printResult(firebaseData1);
            Serial.println("------------------------------------------------------");
            Serial.println();
        }
    }
    else
    {
        Serial.println("--------------------------------------------------------");
        Serial.println("sentResponseDeviceStatus() : updateNode() failed");
        Serial.println("REASON: " + firebaseData1.errorReason());
        Serial.println("--------------------------------------------------------");
        Serial.println();
        return -1;
    }

    return 0;  // done;
}

// =======================================================================
// this function implement for get 'recheck' to Firebase 'dv_staus'to RDS
// ======================================================================
int getRecheckStaus()
{
    String ret = "";

    if (!Firebase.beginStream(firebaseData1, path_dv_staus))
    {
        Serial.println("---------------------------------------------------------");
        Serial.println("getRecheckStaus() : Can't begin stream connection...");
        Serial.println("REASON: " + firebaseData1.errorReason());
        Serial.println("---------------------------------------------------------");
        Serial.println();
        return -1;
    }

    delay(100);
    
    if (!Firebase.readStream(firebaseData1))
    {
        Serial.println("--------------------------------------------------------");
        Serial.println("getRecheckStaus() : Can't read stream data");
        Serial.println("REASON: " + firebaseData1.errorReason());
        Serial.println("-------------------------------------------------------");
        Serial.println();
        return -1;
    }

    if (firebaseData1.streamTimeout())
    {
        Serial.println("Stream timeout, resume streaming...");
        Serial.println();
    }

    if (!Firebase.getString(firebaseData1, path_dv_staus + "/recheck", ret))
    {
        Serial.println("--------------------------------------------------------");
        Serial.println("sentResponseDeviceStatus() : updateNode() failed");
        Serial.println("REASON: " + firebaseData1.errorReason());
        Serial.println("--------------------------------------------------------");
        Serial.println();
        return -1;
    }

    if(ret!= "")
        return ret.toInt();
    else
       return -1;
}

// ======================================================================================
// this function implement for set Payment to Firebase to RDS
// =====================================================================================
int sentPay(String paymentCode, int paymentValue, int passenger_count, String txn_type)
{
    char deviceBuf[4];
    sprintf(deviceBuf, "%04X", device_number); // convert to Hex 4 digits Eg. 1 --> 0001 , 10 ---> 000A
    FirebaseJson json_txn_usage;
    String strDate = convertStrToFileName(dayStamp, 1);
    String strTime = convertStrToFileName(timeStamp, 2);
    // "/txn_usage/TP_yyyymmdd_hhmmss_dddddd.dat"
    String path_txn_id = TX_USAGE + String("/TP_") +
                         String(strDate) + "_" + String(strTime) + "_" +
                         String(deviceBuf[0]) + String(deviceBuf[1]) +
                         String(deviceBuf[2]) + String(deviceBuf[3]) + "_dat";

    Serial.println("sentPay() : sent to node " + path_txn_id);

    if (!Firebase.beginStream(firebaseData2, path_txn_id))
    {
        Serial.println("--------------------------------------------");
        Serial.println("sentPay() : Can't begin stream connection...");
        Serial.println("REASON: " + firebaseData2.errorReason());
        Serial.println("--------------------------------------------");
        Serial.println();
        return -1;
    }

    if (!Firebase.readStream(firebaseData2))
    {
        Serial.println("-----------------------------------------------");
        Serial.println("sentPay() : Can't read stream data");
        Serial.println("REASON: " + firebaseData2.errorReason());
        Serial.println("-----------------------------------------------");
        Serial.println();
        return -1;
    }

    if (firebaseData2.streamTimeout())
    {
        Serial.println("Stream timeout, resume streaming...");
        Serial.println();
    }

    // path for db
    //  txn_usage
    //    |--item_id                             'TP_yyyymmdd_hhmmss_dddddd.dat'
    //        |--device_id      : 'deviceId'       'parameter is IPAddr'
    //        |--device_type    : '15'             'ENV_BUS'
    //        |--location_code  : '0'
    //        |--location
    //              |--latitude   :
    //              |--longitude  :
    //              |--accuracy   :
    //        |--payment
    //              |--passenger_count : '1'
    //              |--passenger_id    : 'uuuuuuuuuuuu'   <read from QR>
    //              |--type            : 'T-Wallet'       <Pay-By-TrueMoney>
    //              |--value           : '20000'          <Satang>
    //        |--txn_date              : <yyyy/mm/dd>
    //        |--txn_time              : <hh:mm:ss>
    //        |--txn_status            : '200'            <200 requests,404 https not found,
    //        |--txn_type              : 'Payment'        <1 Payment,2 void>

    json_txn_usage
        .add("device_id"                    , String(device_ip))
        .add("device_type"                  , "15")
        .add("location_code"                , "0")
        .add("/location/latitude"           , String(dv_location.lat, 7))
        .add("/location/longitude"          , String(dv_location.lon, 7))
        .add("/location/accuracy"           , String(dv_location.accuracy))
        .add("/payment/passenger_count"     , String(passenger_count))
        .add("/payment/passenger_id"        , paymentCode)                  // T-Wallet IDss
        .add("/payment/type"                , "T-Wallet")
        .add("/payment/value"               , String(paymentValue))
        .add("txn_date"                     , dayStamp)
        .add("txn_time"                     , timeStamp)
        .add("txn_status"                   , "200")
        .add("txn_type"                     , txn_type);

    if (Firebase.updateNode(firebaseData2, path_txn_id, json_txn_usage))
    {
        if (more_text)
        {
            Serial.println("-----------------------------------------------");
            Serial.println("sentPay() : updatenode with details");
            Serial.println("PATH: " + firebaseData2.dataPath());
            Serial.println("TYPE: " + firebaseData2.dataType());
            Serial.print("VALUE: ");
            printResult(firebaseData2);
            Serial.println("-----------------------------------------------");
            Serial.println();
        }
    }
    else
    {
        Serial.println("--------------------------------------------------");
        Serial.println("sentPay() : updateNode() failed");
        Serial.println("REASON: " + firebaseData2.errorReason());
        Serial.println("--------------------------------------------------");
        Serial.println();
        return -1;
    }

    return -1;
}