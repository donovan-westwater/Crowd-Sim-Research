using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net.Http.Headers;
using System.Threading;
using UnityEngine;
using UnityEngine.UI;

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
    const float height = 0.01f;
    float tempx = -1;
    float tempy = -1;
    int amount = 0;
    int count = 1;
    float timer = 0;
    string[] lines;
    GameObject[] agents;
    string[] wallFrame;
    GameObject[] walls;
    bool playmode = true;
    int fnum = 0;
    public GameObject prefab;
    public GameObject wall;
    public Text display;
    public Slider rewind;
    public Button pauseButton;
    int modeSwitch = 1;
    Process backsim = new Process();
    void Start()
    {
        UnityEngine.Debug.Log(Application.dataPath);

        backsim.StartInfo.WorkingDirectory = Application.dataPath + "/../OpenGL/";
        backsim.StartInfo.FileName = Application.dataPath + "/../OpenGL/OpenGL.exe";
        backsim.StartInfo.Arguments = "-shash -wallTest";
        backsim.StartInfo.UseShellExecute = true;

        
        backsim.Start();
        while(!File.Exists("frames/frame1.txt")) Thread.Sleep(2000);
        fnum = Directory.GetFiles("./frames").Length;
        lines = File.ReadAllLines("frames/frame1.txt");
        amount = lines.Length;
        positions = new Vector3[amount];
        agents = new GameObject[amount];
        rewind.maxValue = fnum;
        display.text = "Current frame of simulation: " + count;

        SetupWalls();
        //backsim.Start();
    }

    // Update is called once per frame
    void Update()
    {
        //if (!playmode) pauseButton.transform.GetChild(0).transform.GetComponent<Text>().text = "Play";
        //else pauseButton.transform.GetChild(0).transform.GetComponent<Text>().text = "Stop";

        fnum = Directory.GetFiles("./frames").Length;
        rewind.maxValue = fnum;
        display.text = "Current frame of simulation: " + count;
        //rewind.value = count;
        if (playmode) {
            rewind.value = count;
            timer += Time.deltaTime;
            if (Input.GetKeyDown(KeyCode.Space))
            {
                playmode = false;
                pauseButton.transform.GetChild(0).transform.GetComponent<Text>().text = "Play";
            }
            if (timer > 0.05f)
            {
                timer = 0;
        
                string filename = "frames/frame"+count+".txt";
                if (File.Exists(filename))
                {
                    lines = File.ReadAllLines(filename);
                    for (int i = 0; i < amount; i++)
                    {
                        //if (i >= 50) continue;
                        string[] cords = lines[i].Split('|');
                        positions[i] = new Vector3(float.Parse(cords[0]), height, float.Parse(cords[1]));
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
        else //Code to rewind the simulation and look at previous postions of agents
        {
            // display.text = "Current frame of simulation: " + count;
            //rewind.value = count;

            if (!Input.GetKey(KeyCode.LeftArrow) && !Input.GetKey(KeyCode.RightArrow) && !Input.GetKeyDown(KeyCode.Space))
            {
                count = (int)rewind.value;
            }
            else rewind.value = count;
            if (Input.GetKey(KeyCode.LeftArrow)) count--;
            if (Input.GetKey(KeyCode.RightArrow)) count++;
            if (Input.GetKeyDown(KeyCode.Space))
            {
                playmode = true;
                pauseButton.transform.GetChild(0).transform.GetComponent<Text>().text = "Stop";
            }
            if (count < 1) count = 1;
            if (count > fnum) count = fnum;
            string filename = "frames/frame" + count + ".txt";
            if (File.Exists(filename))
            {
                lines = File.ReadAllLines(filename);
                for (int i = 0; i < amount; i++)
                {
                    //if (i >= 50) continue;
                    string[] cords = lines[i].Split('|');
                    positions[i] = new Vector3(float.Parse(cords[0]), 2, float.Parse(cords[1]));
                    if (!agents[i])
                    {
                        agents[i] = Instantiate(prefab);
                        agents[i].GetComponent<AgentVisualizer>().id = i;
                    }
                    agents[i].transform.position = positions[i];

                }  
            }
            
        }
    }
    public void ClickPause()
    {
        playmode = !playmode;
        if (!playmode) pauseButton.transform.GetChild(0).transform.GetComponent<Text>().text = "Play";
        else pauseButton.transform.GetChild(0).transform.GetComponent<Text>().text = "Stop";
    }
    public void switchToScenario(string name)
    {
        backsim.Kill();
        DirectoryInfo di = new DirectoryInfo("./frames");
        foreach (FileInfo file in di.GetFiles())
        {
            file.Delete();
        }

        backsim.StartInfo.Arguments = "-shash -"+name;

        backsim.Start();
        while (!File.Exists("frames/frame1.txt")) Thread.Sleep(2000);
        fnum = Directory.GetFiles("./frames").Length;
        lines = File.ReadAllLines("frames/frame1.txt");
        amount = lines.Length;
        positions = new Vector3[amount];
        //agents = new GameObject[amount];
        rewind.maxValue = fnum;
        display.text = "Current frame of simulation: " + count;
    }
    public void switchVisual()
    {
        bool isActive;
        if(modeSwitch == 1)
        {
            for (int i = 0; i < amount; i++)
            {
                isActive = agents[i].transform.GetChild(0).gameObject.activeSelf;
                agents[i].transform.GetChild(0).gameObject.SetActive(!isActive);
                agents[i].transform.GetChild(1).gameObject.SetActive(!isActive);
            }
            agents[0].GetComponent<AgentVisualizer>().setVisualMode(true);
            modeSwitch--;
        }
        else
        {
            for (int i = 0; i < amount; i++)
            {
                isActive = agents[i].transform.GetChild(0).gameObject.activeSelf;
                agents[i].transform.GetChild(0).gameObject.SetActive(!isActive);
                agents[i].transform.GetChild(1).gameObject.SetActive(!isActive);
            }
            agents[0].GetComponent<AgentVisualizer>().setVisualMode(false);
            modeSwitch++;
        }
        /*
        switch (modeSwitch) {
            case 1:
                for(int i = 0; i < amount; i++)
                {
                    isActive = agents[i].transform.GetChild(0).gameObject.activeSelf;
                    agents[i].transform.GetChild(0).gameObject.SetActive(!isActive);
                    agents[i].transform.GetChild(1).gameObject.SetActive(!isActive);
                }
                modeSwitch++;
                break;
            case 2:
                
                for(int i = 0; i < amount; i++)
                {
                    isActive = agents[i].transform.GetChild(0).gameObject.activeSelf;
                    agents[i].transform.GetChild(0).gameObject.SetActive(!isActive);
                    agents[i].transform.GetChild(1).gameObject.SetActive(!isActive);
                }
                agents[0].GetComponent<AgentVisualizer>().setVisualMode(false);
                modeSwitch++;
                break;
            case 3:
                agents[0].GetComponent<AgentVisualizer>().setVisualMode(true);
                modeSwitch = 1;
                break;
    }
    */
}
    void SetupWalls()
    {
        wallFrame = File.ReadAllLines("walls/frame1.txt");
        string filename = "walls/frame1.txt";
        int wallAmount = wallFrame.Length;
        walls = new GameObject[wallAmount];
        if (File.Exists(filename))
        {
            lines = File.ReadAllLines(filename);
            for (int i = 0; i < wallAmount; i++)
            {
                walls[i] = Instantiate(wall);
                string[] param = lines[i].Split('|');
                walls[i].transform.position = new Vector3(float.Parse(param[0]), wall.transform.position.y, float.Parse(param[1]));
                walls[i].transform.localScale = new Vector3(float.Parse(param[3]),walls[i].transform.localScale.y, float.Parse(param[2]));
                
            }
            
        }
    }
    private void OnApplicationQuit()
    {
        backsim.Kill();
        DirectoryInfo di = new DirectoryInfo("./frames");
        foreach (FileInfo file in di.GetFiles())
        {
            file.Delete();
        }
    }
}
