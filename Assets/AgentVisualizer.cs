using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AgentVisualizer : MonoBehaviour
{
    // Start is called before the first frame update
   
    public Material circle;
    public GameObject cylinder;
    public int id;
    static Vector4[] array = new Vector4[300];
    public static bool visualMode = false;
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        array[id] = this.transform.position;
        
        //Turns on cylinders of agents when visualMode is true
        if (visualMode)
        {
            
            cylinder.SetActive(true);
        }else cylinder.SetActive(false);
       
    }

    public void setVisualMode(bool b)
    {
        visualMode = b;
        if (!b)
        {
            Vector4[] clear = new Vector4[300];
            for (int i = 0; i < 300; i++)
            {
                clear[i] = new Vector4(9999, 9999, 9999, 9999);
            }
            circle.SetVectorArray("_aLocs", clear);
        }
    }
}
