using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AgentVisualizer : MonoBehaviour
{
    // Start is called before the first frame update
   
    public Material circle;
    public int id;
    static Vector4[] array = new Vector4[300];
    public static bool visualMode = true;
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        array[id] = this.transform.position;
        //circle.SetFloat("_AgentPosX", this.transform.position.x);
        //circle.SetFloat("_AgentPosY", this.transform.position.z);
        if(visualMode) circle.SetVectorArray("_aLocs", array);
        //circle.SetVector("_aLocs" + id, new Vector3(this.transform.position.x,-1, this.transform.position.z));
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
