using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class SystemScripts : MonoBehaviour {

    public TextMeshProUGUI longitude;
    public TextMeshProUGUI latitude;

    IEnumerator Start ()
    {
        Screen.orientation = ScreenOrientation.LandscapeRight;
        Debug.Log("Inside IEnumerator");

        //if (!Input.location.isEnabledByUser)
        //{
        //    Debug.Log("Location services disabled by user.");
        //    yield break;
        //}

        Input.location.Start();
        Debug.Log("Force starting user location");

        int maxWait = 20; // wait until service initializes

        while (Input.location.status == LocationServiceStatus.Initializing && maxWait > 0)
        {
            yield return new WaitForSeconds(1);
            maxWait--;
        }

        if (maxWait < 1)
        {
            Debug.Log("GNSS timed out...");
            yield break;
        }
        
        if (Input.location.status == LocationServiceStatus.Failed)
        {
            Debug.Log("Location services failed");
            yield break;
        }
        else
        {
            Debug.Log("Location service functioning!");
            longitude.text = Input.location.lastData.longitude.ToString();
            latitude.text = Input.location.lastData.latitude.ToString();
        }
    }
	
    public void exitApp ()
    {
        Input.location.Stop();
        Application.Quit();
    }

}
