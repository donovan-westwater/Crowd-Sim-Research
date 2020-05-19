using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AgentVisualizer : MonoBehaviour
{
    // Start is called before the first frame update
    /*TODO:
     * Create an array of agents based on the amount of lines found in the first text file
     * Each update, read the values from the frame text, and then assign them to agents in the array
     * When there are no more files to read, stop the simulation
     * */
    public Material circle;
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        circle.SetFloat("_AgentPosX", this.transform.position.x);
        circle.SetFloat("_AgentPosY", this.transform.position.z);
    }
}
