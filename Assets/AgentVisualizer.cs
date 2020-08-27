using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AgentVisualizer : MonoBehaviour
{
    // Start is called before the first frame update
   
    
    public GameObject cylinder;
    public GameObject facing;
    public int id;
   
    public static bool visualMode = false; //Are the agents cylinders or models
    void Start()
    {
        //Gives the agent's cylinders a color based on the id
        if (id % 5 == 0) cylinder.GetComponent<Renderer>().material.SetColor("_Color", Color.red);
        else if(id % 2 == 0) cylinder.GetComponent<Renderer>().material.SetColor("_Color", Color.cyan);
        else if(id % 3 == 0) cylinder.GetComponent<Renderer>().material.SetColor("_Color", Color.yellow);
    }

    // Update is called once per frame
    void Update()
    {


        //Turns on cylinders of agents when visualMode is true
        if (visualMode)
        {
            facing.SetActive(true);
            cylinder.SetActive(true);
        }
        else
        {
            facing.SetActive(false);
            cylinder.SetActive(false); //Turns cylinders off when false
        }
    }

    public void setVisualMode(bool b)
    {
        visualMode = b;
      
    }
}
