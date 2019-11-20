using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

using TechTweaking.Bluetooth;

public class LetsGetBluetooth : MonoBehaviour {

    private BluetoothDevice device;
    public Text statusText;
    public Text sizeOfMessage; // get message size
    public Text messageZero; // message at index 0
    public Text messageOne; // message at index 1
    public Text messageTwo; // message at index 2

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

    // Use this for initialization
    void Start() {
		
	}
	
	// Update is called once per frame
	void Update() {
		
	}
}
