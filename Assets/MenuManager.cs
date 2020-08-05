using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MenuManager : MonoBehaviour
{
    // Start is called before the first frame update
    public GameObject SimMenu;
    public GameObject InSimMenu;
   
    public void goToSimMenu()
    {
        InSimMenu.SetActive(false);
        SimMenu.SetActive(true);
    }
    public void goToInSimMenu()
    {
        Debug.Log("BUTTON PRESSED!");
        InSimMenu.SetActive(true);
        SimMenu.SetActive(false);
    }
}
