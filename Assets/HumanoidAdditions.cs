using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HumanoidAdditions : MonoBehaviour
{
    // Start is called before the first frame update
    
    public Animator animator; //Animator compoetnet of the agent
    Vector3 oldPos; //Old position of the agent
    Vector3 dir; //new facing of the agent
    float angle; //angle of current facing
    int updateCount = 0;
    float avgSpeed = 0; //average speed of the agent
    float[] prevSpeeds = { -1, -1, -1,-1}; //Array of previous speeds, used to make average speed
    int prevSpeedCounter = 0;
    void Start()
    {
        oldPos = this.transform.position;
        
        animator = this.transform.GetComponent<Animator>();
        
        angle = 0;
    }

    // Update is called once per frame
    void Update()
    {
        //Controls the rotation of the agent
        dir = this.transform.position - oldPos;
        dir.y = 0;
        if(dir.magnitude >= 0.01f) { 
            Vector3 temp = new Vector3(Mathf.Sin(angle*Mathf.Deg2Rad), 0, Mathf.Cos(angle*Mathf.Deg2Rad));
            float angleTemp = Mathf.Acos(Vector3.Dot(dir, temp) / (dir.magnitude * temp.magnitude)); //stored as rad
            Vector3 product = Vector3.Cross(dir.normalized, temp.normalized);
            if (product.y <= 0 && angleTemp > Mathf.Abs(0.01f))
            {
                angle += angleTemp*Mathf.Rad2Deg;
            }
            else if (product.y > 0 && angleTemp > Mathf.Abs(0.01f))
            {
                angle -= angleTemp * Mathf.Rad2Deg;
            }
        }
        
        //Chnage the y part of hte euler angles to rotate the whole model
        //Get the angle by using the "true" north (find the vector for when rotaion y == 0)
        this.transform.eulerAngles = new Vector3(this.transform.eulerAngles.x, angle, this.transform.eulerAngles.z);
        
        //Controls which animation the model plays
        //Gets speed and then stores it * 100 to have better percision
        Vector3 speed = this.transform.position - oldPos;
        //Stores the value as a previous speed, goes back to start when it counter goes past the max index
        prevSpeeds[prevSpeedCounter] = speed.magnitude * 100;
        prevSpeedCounter++;
        if(prevSpeedCounter > 3)
        {
            prevSpeedCounter = 0;
        }
        float numOfSpeeds = 0;
        float totalSpeeds = 0;
        //Averages the prvious speeds
        for(int i = 0; i < 4; i++)
        {
            if (prevSpeeds[i] == -1) continue;
            numOfSpeeds++;
            totalSpeeds += prevSpeeds[i];
        }
        float avgS = totalSpeeds / numOfSpeeds;
        //Sends speed to animator to evaluate
        animator.SetFloat("speed", speed.magnitude*100);
        bool idlemode = animator.GetBool("idlemode");
        float curSpeed = (int)(speed.magnitude*100);
        if (avgS <= 1) animator.SetBool("idlemode", true);
        else animator.SetBool("idlemode", false);

        oldPos = this.transform.position;

        avgSpeed += speed.magnitude;
        updateCount += 1;
        
    }
    //Prints the average speed of the agent overall when sim is closed
    private void OnApplicationQuit()
    {
        avgSpeed /= updateCount;
        Debug.Log("Average speed: " + avgSpeed);
    }
}
