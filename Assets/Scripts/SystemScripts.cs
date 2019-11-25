using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SystemScripts : MonoBehaviour {

	// Use this for initialization
	void Start () {
        Screen.orientation = ScreenOrientation.Landscape;
    }
	
    public void exitApp ()
    {
        Application.Quit();
    }

	// Update is called once per frame
	void Update () {
		
	}
}
