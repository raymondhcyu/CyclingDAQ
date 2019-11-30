using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

using TechTweaking.Bluetooth;
using TMPro; // for textmeshpro

using System;
using System.IO;
using System.Text;

public class LetsGetBluetooth : MonoBehaviour
{
    private BluetoothDevice device;
    private string fileName;
    private List<string[]> rowData = new List<string[]>(); // dynamic array; https://sushanta1991.blogspot.com/2015/02/how-to-write-data-to-csv-file-in-unity.html 
    private const int rowDataSize = 10; // change to match data size to save to CSV (not BT packet)
    private const int BTDataSize = 8;
    private string[] rowDataTemp = new string[rowDataSize]; // temp array for writing
    // Implement try catch for out of memory? https://docs.microsoft.com/en-us/dotnet/api/system.outofmemoryexception?view=netframework-4.8
    private string delimiter = ",";
    private string filePath = "";

    public TextMeshProUGUI statusText;
    public TextMeshProUGUI sizeOfMessage; // get message size
    public TextMeshProUGUI xAccelDisplay;
    public TextMeshProUGUI yAccelDisplay;
    public TextMeshProUGUI zAccelDisplay;
    public TextMeshProUGUI steeringDisplay; // same as yaw
    public TextMeshProUGUI pitchDisplay;
    public TextMeshProUGUI rollDisplay;
    public TextMeshProUGUI brakeDisplay;

    public TextMeshProUGUI testPointText1;
    public TextMeshProUGUI testPointText2;
    public TextMeshProUGUI testPointText3;

    public SystemScripts phoneSystemScripts; // get GPS data

    void Awake()
    {
        BluetoothAdapter.askEnableBluetooth();

        device = new BluetoothDevice();
        device.Name = "ESP32test";
    }

    void Start()
    {
        // Change to have timestamp
        fileName = "/bikingProgramData57.csv";

        // Init titles of CSV
        rowDataTemp = new string[rowDataSize];
        rowDataTemp[0] = "Time";
        rowDataTemp[1] = "X Accel";
        rowDataTemp[2] = "Y Accel";
        rowDataTemp[3] = "Z Accel";
        rowDataTemp[4] = "Yaw";
        rowDataTemp[5] = "Pitch";
        rowDataTemp[6] = "Roll";
        rowDataTemp[7] = "Brake";
        rowDataTemp[8] = "Longitude";
        rowDataTemp[9] = "Latitude";
        rowData.Add(rowDataTemp);
    }

    public void connect()
    {
        device.connect();
        if (device != null)
            statusText.text = "CONNECTED";
    }

    public void disconnect()
    {
        statusText.text = "DISCONNECTING...";

        try
        {
            filePath = Application.persistentDataPath + fileName;

            if (!File.Exists(filePath))
            {
                Debug.Log("Saving file to: " + filePath);

                // Save data from dynamic array to persistentDataPath
                string[][] output = new string[rowData.Count][]; // init 2D array to convert to csv

                for (int i = 0; i < output.Length; i++)
                    output[i] = rowData[i];

                int length = output.GetLength(0);
                StringBuilder sb = new StringBuilder();

                for (int index = 0; index < length; index++)
                    sb.AppendLine(string.Join(delimiter, output[index]));

                StreamWriter outStream = File.CreateText(filePath);
                outStream.WriteLine(sb);
                outStream.Close();
            }
            else
                Debug.Log("File already exists at: " + filePath);

        }
        catch (System.Exception e)
        {
            Debug.Log(e);
        }

        // Close app
        device.close();
        statusText.text = "DISCONNECTED";
    }

    public void requestData()
    {
        Debug.Log("Getting BT data...");

        if (device != null)
            device.send(System.Text.Encoding.ASCII.GetBytes("Send me data\n"));
    }

    // Update is called once per frame
    void Update()
    {
        if (device.IsReading)
        {
            byte[] msg = device.read();

            if ((msg != null) && (msg[0] == 255))
            {
                Debug.Log("Processing message.");

                // Display updates to UI
                statusText.text = "MSG RECEIVED: " + msg[1].ToString() + 
                    msg[2].ToString() + msg[3].ToString() + msg[4].ToString() + 
                    msg[5].ToString() + msg[6].ToString() + msg[7].ToString();
                sizeOfMessage.text = "MSG SIZE: " + msg.Length;

                // Log data to dynamic array
                rowDataTemp = new string[rowDataSize];

                // Log time
                // rowDataTemp[0] = System.DateTime.Now.ToLongDateString();
                rowDataTemp[0] = "0";

                // Acceleration data, start from 1 to skip start byte
                for (int i = 1; i < 4; i++)
                    rowDataTemp[i] = msg[i].ToString();

                // Steering (yaw) data
                rowDataTemp[4] = msg[4].ToString();

                // Pitch and roll data
                rowDataTemp[5] = msg[1].ToString(); // repeat x-axis accel for now
                rowDataTemp[6] = msg[2].ToString(); // repeat y-axis accel for now

                // Brake data
                rowDataTemp[7] = msg[5].ToString();

                // Every time receive message also get GPS data
                //Debug.Log("GPS data: " + phoneSystemScripts.GetLng().ToString() + "\n" + phoneSystemScripts.GetLat().ToString());
                phoneSystemScripts.longitudeDisplay.text = phoneSystemScripts.GetLng().ToString();
                phoneSystemScripts.latitudeDisplay.text = phoneSystemScripts.GetLat().ToString();
                rowDataTemp[8] = phoneSystemScripts.GetLng().ToString("R");
                rowDataTemp[9] = phoneSystemScripts.GetLat().ToString("R");

                // Display messages to UI
                {
                    xAccelDisplay.text = msg[1].ToString();
                    yAccelDisplay.text = msg[2].ToString();
                    zAccelDisplay.text = msg[3].ToString();
                    steeringDisplay.text = msg[4].ToString();
                    brakeDisplay.text = msg[5].ToString();
                    pitchDisplay.text = msg[1].ToString();
                    rollDisplay.text = msg[2].ToString();
                }

                // Write to data
                rowData.Add(rowDataTemp);
            }
        }
    }
}