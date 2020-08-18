using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MenuManager : MonoBehaviour
{
    // Start is called before the first frame update
    public GameObject SimMenu;
    public GameObject InSimMenu;
   //switches from the menu when in the simulation to sceanrio select
    public void goToSimMenu()
    {
        InSimMenu.SetActive(false);
        SimMenu.SetActive(true);
    }
    //swiches from sceanrio select to simulation menu
    public void goToInSimMenu()
    {
        Debug.Log("BUTTON PRESSED!");
        InSimMenu.SetActive(true);
        SimMenu.SetActive(false);
    }
}
