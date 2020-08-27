using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AgentVisualizer : MonoBehaviour
{
    // Start is called before the first frame update
   
    Material circle; //Unused
    public GameObject cylinder;
    public int id;
    static Vector4[] array = new Vector4[300]; //Unused
    public static bool visualMode = false; //Are the agents cylinders or models
    void Start()
    {
        //Gives the agent's cylinders a color based on the id
        if (id % 5 == 0) cylinder.GetComponent<Renderer>().material.SetColor("_Color", Color.red);
        else if(id % 2 == 0) cylinder.GetComponent<Renderer>().material.SetColor("_Color", Color.blue);
        else if(id % 3 == 0) cylinder.GetComponent<Renderer>().material.SetColor("_Color", Color.yellow);
    }

    // Update is called once per frame
    void Update()
    {
        array[id] = this.transform.position;
        
        //Turns on cylinders of agents when visualMode is true
        if (visualMode)
        {
            
            cylinder.SetActive(true);
        }else cylinder.SetActive(false); //Turns cylinders off when false
       
    }

    public void setVisualMode(bool b)
    {
        visualMode = b;
        if (!b)
        {
            //Unused (Delete this section)
            Vector4[] clear = new Vector4[300];
            for (int i = 0; i < 300; i++)
            {
                clear[i] = new Vector4(9999, 9999, 9999, 9999);
            }
            //circle.SetVectorArray("_aLocs", clear);
        }
    }
}
