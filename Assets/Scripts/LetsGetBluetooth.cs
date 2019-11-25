using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

using TechTweaking.Bluetooth;
using TMPro;

using System.IO;

public class LetsGetBluetooth : MonoBehaviour {

    private BluetoothDevice device;
    private string fileName;
    private List<int> data1 = new List<int>(); // dynamic array
    // Implement try catch for out of memory? https://docs.microsoft.com/en-us/dotnet/api/system.outofmemoryexception?view=netframework-4.8

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

    void Awake()
    {
        BluetoothAdapter.askEnableBluetooth();

        device = new BluetoothDevice();
        device.Name = "ESP32test";

        fileName = "bikingProgramData.txt";
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
        // Save data from dynamic array to persistentDataPath
        List<string> data1String = data1.ConvertAll<string>(x => x.ToString()); // convert from int to string
        testPointText1.text = "1";
        string[] data1StringArray = data1String.ToArray(); // convert from string list to string array
        testPointText1.text = "2";
        File.WriteAllLines(Application.persistentDataPath + "/" + fileName, data1StringArray);
        testPointText1.text = "3";

        // Close app
        statusText.text = "DISCONNECTING...";
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

                // Log data to dynamic array
                data1.Add(msg[1]);
            }
        }
	}
}
