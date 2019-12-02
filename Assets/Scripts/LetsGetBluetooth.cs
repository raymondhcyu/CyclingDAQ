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
    private string timeStamp = "";
    private int correctPitch;
    private int correctRoll;
    private int correctBrake;

    public TextMeshProUGUI statusText;
    public TextMeshProUGUI sizeOfMessage; // get message size
    public TextMeshProUGUI xAccelDisplay;
    public TextMeshProUGUI yAccelDisplay;
    public TextMeshProUGUI zAccelDisplay;
    public TextMeshProUGUI steeringDisplay; // same as yaw
    public TextMeshProUGUI pitchDisplay;
    public TextMeshProUGUI rollDisplay;
    public TextMeshProUGUI brakeDisplay;
    public TextMeshProUGUI brakeWarning;

    public SystemScripts phoneSystemScripts; // get GPS data

    void Awake()
    {
        BluetoothAdapter.askEnableBluetooth();

        device = new BluetoothDevice();
        device.Name = "ESP32test";
    }

    void Start()
    {
    }

    public void connect()
    {
        device.connect();
        if (device != null)
        {
            statusText.text = "CONNECTED";
            statusText.color = new Color(0, 255, 0, 255);
        }

        // Set filename with timestamp
        timeStamp = System.DateTime.Now.ToString("yyyy_MM_dd_HH_mm_ss");
        fileName = "/bikingData_" + timeStamp + ".csv";

        // Init titles of CSV
        rowDataTemp = new string[rowDataSize];
        rowDataTemp[0] = "Time";
        rowDataTemp[1] = "X Accel";
        rowDataTemp[2] = "Y Accel";
        rowDataTemp[3] = "Z Accel";
        rowDataTemp[4] = "Empty";
        rowDataTemp[5] = "Pitch";
        rowDataTemp[6] = "Roll";
        rowDataTemp[7] = "Brake";
        rowDataTemp[8] = "Longitude";
        rowDataTemp[9] = "Latitude";
        rowData.Add(rowDataTemp);
    }

    public void disconnect()
    {
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
        statusText.text = "DISCONNECTED & SAVED";
        statusText.color = new Color(0, 179, 255, 255);
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
                statusText.color = new Color(0, 255, 0, 255);
                //statusText.text = msg[1].ToString() +
                //    msg[2].ToString() + msg[3].ToString() + msg[4].ToString() +
                //    msg[5].ToString() + msg[6].ToString() + msg[7].ToString();
                sizeOfMessage.text = "MSG SIZE: " + msg.Length;

                // Apply corrections and brake warning
                correctPitch = (-1 * (msg[3] - 127) * 3) - 20;
                correctRoll = Mathf.Abs(msg[1] - 127) * 2;
                correctBrake = (int)(3.48 * Math.Exp(0.0279 * msg[5])); // cast as int

                if ((msg[5] > 50) && (correctRoll > 10)) // purposely leave this as comparing raw brake byte
                {
                    Debug.Log("Brake warning.");
                    brakeWarning.text = "WARNING: RELEASE BRAKE";
                    brakeWarning.color = new Color(255, 0, 0, 255);
                }
                else
                    brakeWarning.text = "";

                // Log data to dynamic array
                rowDataTemp = new string[rowDataSize];

                // Log time
                timeStamp = System.DateTime.Now.ToString("yyyy_MM_dd_HH_mm_ss_ff");
                rowDataTemp[0] = timeStamp;

                // Acceleration data, start from 1 to skip start byte
                for (int i = 1; i < 4; i++)
                    rowDataTemp[i] = msg[i].ToString();

                // Steering (yaw) data
                rowDataTemp[4] = msg[4].ToString();

                // Pitch and roll data
                rowDataTemp[5] = correctPitch.ToString();
                rowDataTemp[6] = correctRoll.ToString();

                // Brake data
                rowDataTemp[7] = correctBrake.ToString();

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
                    brakeDisplay.text = correctBrake.ToString();
                    pitchDisplay.text = correctPitch.ToString();
                    rollDisplay.text = correctRoll.ToString();
                }

                // Write to data
                rowData.Add(rowDataTemp);
            }
        }
    }
}