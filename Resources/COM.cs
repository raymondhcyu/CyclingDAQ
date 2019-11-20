using UnityEngine;
using System.Collections;
using System.IO.Ports;
using System;
using System.Threading;

public class COM : MonoBehaviour {
	
	public SerialPort sp;
	public int bytes;
	public Thread serialThread;
	public float x1,y1,z1;
	public bool readyToMove = false;
	public float prevY = 0.0f;

	void Start () {
		sp = new SerialPort("COM3", 19200);
	}

	void parseValues(string av) {

		string[] split = av.Split (',');
	    x1 = float.Parse (split [2]);
		y1 = float.Parse (split [3]);
		z1 = float.Parse(split[4]);
		readyToMove = true;
	}


	void moveObj(float x, float y, float z) {
		int speed = 10;
		Vector3 move = Vector3.zero ;
		move.y = y;
		move.Normalize();
		move.y = Mathf.Lerp(move.y,prevY,speed*Time.deltaTime);
		transform.Translate(Vector3.up * (move.y * speed) * Time.deltaTime,Space.World);
		readyToMove = false;
		prevY = move.y;
	}

	
	 void recData() {
		if ((sp != null) && (sp.IsOpen)) {
			byte tmp;
			string data = "";
			string avalues="";
			tmp = (byte) sp.ReadByte();
			while(tmp !=255) {
				data+=((char)tmp);
				tmp = (byte) sp.ReadByte();
				if((tmp=='>') && (data.Length > 30)){
					avalues = data;
					parseValues(avalues);
					data="";
				}
			}
		}
	}


	void connect() {
		Debug.Log ("Connection started");
		try {
			sp.Open();
			sp.ReadTimeout = 400;
			sp.Handshake = Handshake.None;
			serialThread = new Thread(recData);
			serialThread.Start ();
			Debug.Log("Port Opened!");
		}catch (SystemException e)
		{
			Debug.Log ("Error opening = "+e.Message);
		}
	
	}


	void Update () { 

		 if (Input.GetKeyDown ("x")) {
			Debug.Log("Connection establishing...");
			connect ();
		}

		if (readyToMove == true) {
			moveObj(x1,y1,z1);
		}

	}


}