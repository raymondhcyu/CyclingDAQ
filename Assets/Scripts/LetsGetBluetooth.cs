using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

using TechTweaking.Bluetooth;
using TMPro;

using System;
using System.IO;
using System.Text;

public class LetsGetBluetooth : MonoBehaviour {

    private BluetoothDevice device;
    private string fileName;
    private List<string[]> rowData = new List<string[]>(); // dynamic array; https://sushanta1991.blogspot.com/2015/02/how-to-write-data-to-csv-file-in-unity.html 
    private const int rowDataSize = 4; // change to match data size
    private string[] rowDataTemp = new string[rowDataSize]; // temp array for writing
    // Implement try catch for out of memory? https://docs.microsoft.com/en-us/dotnet/api/system.outofmemoryexception?view=netframework-4.8
    private string delimiter = ",";
    private string filePath = "";

    public TextMeshProUGUI statusText;
    public TextMeshProUGUI sizeOfMessage; // get message size
    public TextMeshProUGUI messageZero; // message at index 0
    public TextMeshProUGUI messageOne; // message at index 1
    public TextMeshProUGUI messageTwo; // message at index 2
    public TextMeshProUGUI messageThree; // message at index 3
    public TextMeshProUGUI messageFour; // message at index 4

    public TextMeshProUGUI testPointText1;
    public TextMeshProUGUI testPointText2;
    public TextMeshProUGUI testPointText3;
    public TextMeshProUGUI testPointText4;

    void Awake()
    {
        BluetoothAdapter.askEnableBluetooth();

        device = new BluetoothDevice();
        device.Name = "ESP32test";
    }

    void Start()
    {
        fileName = "/bikingProgramData3.txt";

        // Init titles of CSV
        rowDataTemp = new string[rowDataSize];
        rowDataTemp[0] = "Title0";
        rowDataTemp[1] = "Title1";
        rowDataTemp[2] = "Title2";
        rowDataTemp[3] = "Title3";
        rowData.Add(rowDataTemp);
    }

    public void connect()
    {
        statusText.text = "CONNECTING...";
        device.connect();
        if (device != null)
            statusText.text = "CONNECTED";
    }

    public void disconnect()
    {
        statusText.text = "DISCONNECTING...";

        filePath = Application.persistentDataPath + fileName;
        string[] testData = { "Test1", "Test2", "Test3" };
        File.WriteAllLines(filePath, testData);

        testPointText1.text = "Maybe wrote data to file";

        testPointText2.text = filePath;

        // https://gamedevelopment.tutsplus.com/tutorials/how-to-save-and-load-your-players-progress-in-unity--cms-20934

        //// Save data from dynamic array to persistentDataPath
        //string[][] output = new string[rowData.Count][]; // init 2D array to convert to csv

        //testPointText1.text = rowData.Count.ToString();

        //for (int i = 0; i < output.Length; i++)
        //{
        //    output[i] = rowData[i];
        //    //testPointText1.text = "Wrote " + rowData[i][i] + " to output: " + output[i][i];
        //}

        //int length = output.GetLength(0);
        //StringBuilder sb = new StringBuilder();

        ////testPointText2.text = length.ToString();

        //for (int index = 0; index < length; index++)
        //    sb.AppendLine(string.Join(delimiter, output[index]));

        //filePath = Application.persistentDataPath + fileName;

        //StreamWriter outStream = File.CreateText(filePath);
        //outStream.WriteLine(sb);
        //outStream.Close();

        {
        /* 
        List<string> data1String = rowData.ConvertAll<string>(x => x.ToString()); // convert from int/float to string
        testPointText1.text = "1";
        string[] data1StringArray = rowData.ToArray(); // convert from string list to string array
        testPointText1.text = "2";
        File.WriteAllLines(Application.persistentDataPath + "/" + fileName, data1StringArray);
        testPointText1.text = "3";
        */
        }

        // Close app
        device.close();
        statusText.text = "DISCONNECTED";
    }

    public void requestData()
    {
        if (device != null)
            device.send(System.Text.Encoding.ASCII.GetBytes("Send me data\n"));
    }
	
	// Update is called once per frame
	void Update() {
		if (device.IsReading)
        {
            byte[] msg = device.read();

            if ((msg != null) && (msg[0] == 255))
            {
                // Display updates to UI
                statusText.text = "MSG RECEIVED: " + msg[1].ToString() + msg[2].ToString() + msg[3].ToString();
                //statusText.text = "MSG RECEIVED: " + foreach (byte item in msg) {item.ToString()};
                sizeOfMessage.text = "MSG SIZE: " + msg.Length;
                //messageZero.text = msg[0].ToString(); // start byte 255
                messageOne.text = msg[1].ToString();
                messageTwo.text = msg[2].ToString();
                messageThree.text = msg[3].ToString();
                //messageFour.text = msg[4].ToString();

                // Add calibration algorithms here?

                //// Log data to dynamic array
                //rowDataTemp = new string[rowDataSize];
                //for (int i = 1; i < rowDataSize; i++)
                //{
                //    rowDataTemp[i] = msg[i].ToString();
                //}
                //rowData.Add(rowDataTemp);

                //testPointText3.text = "Added " + rowDataTemp[1] + rowDataTemp[2] + rowDataTemp[3];

                //foreach (string[] item in rowData)
                //    foreach (string subItem in item)
                //        testPointText4.text = subItem;

            }
        }
	}
}
