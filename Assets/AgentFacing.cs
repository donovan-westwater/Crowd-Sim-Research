using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AgentFacing : MonoBehaviour
{
    //Should probably integrate this code into the Agent Player
    public GameObject aimRet;
    Vector3 oldPos;
    Vector3 facing;

    // Start is called before the first frame update
    void Start()
    {
        oldPos = this.transform.position;
        facing = aimRet.transform.position - transform.position;
        facing.y = 0;//transform.position.y;
    }

    // Update is called once per frame
    void Update()
    {
        Vector3 newDir = new Vector3(oldPos.x - transform.position.x,0, oldPos.z - transform.position.z);//transform.position.y for y param
        newDir = -newDir;
        float angleTemp = Mathf.Acos(Vector3.Dot(newDir, facing) / (newDir.magnitude * facing.magnitude)); //Is in Radians!
        if (oldPos != transform.position)
        {
            oldPos = transform.position;
        }
        Vector3 product = Vector3.Cross(newDir.normalized, facing.normalized);
        if (Vector3.Cross(newDir.normalized, facing.normalized).y <= 0 && angleTemp > Mathf.Abs(0.01f))
        {
            aimRet.transform.RotateAround(transform.position, new Vector3(0, 1, 0), angleTemp*Mathf.Rad2Deg); //* Mathf.Rad2Deg*Time.deltaTime);
            facing = aimRet.transform.position - transform.position;
            facing.y = 0; //transform.position.y;
        }
        else if (Vector3.Cross(newDir.normalized, facing.normalized).y > 0 && angleTemp > Mathf.Abs(0.01f))
        {
            aimRet.transform.RotateAround(transform.position, new Vector3(0, 1, 0), -angleTemp * Mathf.Rad2Deg); //* Mathf.Rad2Deg * Time.deltaTime);
            facing = aimRet.transform.position - transform.position;
            facing.y = 0; //transform.position.y;
        }
        //if (Input.GetKey(KeyCode.D)) transform.position += new Vector3(10*Time.deltaTime,0,1*Time.deltaTime); 
        
    }
}
