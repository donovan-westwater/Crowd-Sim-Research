using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;

public class AgentPlayer : MonoBehaviour
{
    // Start is called before the first frame update
    /*
     * What this does: Take record made in the crowd sim side of things and play it out in unity
     * Broken down into steps:
    *       * Create an array of agents based on the amount of lines found in the first text file
    *       * Each update, read the values from the frame text, and then assign them to agents in the array
    *       * When there are no more files to read, stop the simulation
    * */
    Vector3[] positions;
    float tempx = -1;
    float tempy = -1;
    int amount = 0;
    int count = 1;
    float timer = 0;
    string[] lines;
    GameObject[] agents;
    public GameObject prefab;
    void Start()
    {
        lines = File.ReadAllLines("frames/frame1.txt");
        amount = lines.Length;
        positions = new Vector3[amount];
        agents = new GameObject[amount];
    }

    // Update is called once per frame
    void Update()
    {
        timer += Time.deltaTime;
        if (timer > 0.05f)
        {
            timer = 0;
        
            string filename = "frames/frame"+count+".txt";
            if (File.Exists(filename))
            {
                lines = File.ReadAllLines(filename);
                for (int i = 0; i < amount; i++)
                {
                    if (i >= 50) continue;
                    string[] cords = lines[i].Split('|');
                    positions[i] = new Vector3(float.Parse(cords[0]), 2, float.Parse(cords[1]));
                    if (!agents[i])
                    {
                        agents[i] = Instantiate(prefab);
                        agents[i].GetComponent<AgentVisualizer>().id = i;
                    }
                    agents[i].transform.position = positions[i];

                }
                count++;
            }
        }
    }
}
