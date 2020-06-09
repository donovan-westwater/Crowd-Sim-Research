using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AgentFacing : MonoBehaviour
{
    //Should probably integrate this code into the Agent Player
    public GameObject aimRet;
    Vector3 oldPos;
    Vector2 facing;

    // Start is called before the first frame update
    void Start()
    {
        oldPos = this.transform.position;
        facing = new Vector2(0, 1);
    }

    // Update is called once per frame
    void Update()
    {
        Vector3 newDir = new Vector3(oldPos.x - transform.position.x, oldPos.y - transform.position.y);
        float angleTemp = Mathf.Acos(Vector3.Dot(newDir, facing) / (newDir.magnitude * facing.magnitude)); //Is in Radians!
        oldPos = transform.position;
       
        if (Vector3.Cross(facing.normalized, newDir.normalized).z < 0 && angleTemp > Mathf.Abs(0.01f))
        {
            aimRet.transform.RotateAround(transform.position, new Vector3(0, 1, 0), angleTemp * Mathf.Rad2Deg);
            facing = newDir;
        }
        else if (Vector3.Cross(facing.normalized, newDir.normalized).z > 0 && angleTemp > Mathf.Abs(0.01f))
        {
            aimRet.transform.RotateAround(transform.position, new Vector3(0, 1, 0), -angleTemp * Mathf.Rad2Deg);
            facing = newDir;
        }
        if (Input.GetKey(KeyCode.D)) transform.position += new Vector3(10*Time.deltaTime,0,0); 
        
    }
}
