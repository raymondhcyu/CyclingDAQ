using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

using TechTweaking.Bluetooth;
using TMPro;

public class LetsGetBluetooth : MonoBehaviour {

    private BluetoothDevice device;
    public TextMeshProUGUI statusText;
    public TextMeshProUGUI sizeOfMessage; // get message size
    public TextMeshProUGUI messageZero; // message at index 0
    public TextMeshProUGUI messageOne; // message at index 1
    public TextMeshProUGUI messageTwo; // message at index 2
    public TextMeshProUGUI messageThree; // message at index 3
    public TextMeshProUGUI messageFour; // message at index 4

    void Awake()
    {
        BluetoothAdapter.askEnableBluetooth();

        device = new BluetoothDevice();
        device.Name = "ESP32test";
    }

    public void connect()
    {
        statusText.text = "STATUS: CONNECTING...";

        device.connect();
    }

    public void disconnect()
    {
        device.close();
    }

    public void startTransmission()
    {
        if (device != null)
            device.send(System.Text.Encoding.ASCII.GetBytes("Start\n"));
    }
	
	// Update is called once per frame
	void Update() {
		if (device.IsReading)
        {
            byte[] msg = device.read();

            if (msg != null)
            {
                statusText.text = "MSG RECEIVED: " + msg[0].ToString() + msg[1].ToString() + msg[2].ToString();
                //statusText.text = "MSG RECEIVED: " + foreach (byte item in msg) {item.ToString()};
                sizeOfMessage.text = "MSG SIZE: " + msg.Length;
                messageZero.text = msg[0].ToString();
                messageOne.text = msg[1].ToString();
                messageTwo.text = msg[2].ToString();
                //messageThree.text = msg[3].ToString();
                //messageFour.text = msg[4].ToString();
            }
        }
	}
}
