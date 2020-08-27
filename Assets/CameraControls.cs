using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraControls : MonoBehaviour
{
    // Start is called before the first frame update
    Camera camera;
    void Start()
    {
        camera = this.GetComponent<Camera>();
    }

    // Update is called once per frame
    void Update()
    {
        //Zooms the camera in and out
        camera.orthographicSize += Input.mouseScrollDelta.y;
        //prevents the zoom from going too far out or in
        if (camera.orthographicSize < 1) camera.orthographicSize = 1;
        if (camera.orthographicSize > 25) camera.orthographicSize = 25;

        Vector2 cameraCoords = new Vector2(camera.transform.position.x, camera.transform.position.z);
        if (Vector2.Distance(cameraCoords, new Vector2(0, 0)) <= 10)
        {
            //Controls to move the camera
            if (Input.GetKey(KeyCode.W)) {
                camera.transform.position += new Vector3(0,0,1) * Time.deltaTime;
            }
            else if (Input.GetKey(KeyCode.S))
            {
                camera.transform.position -= new Vector3(0, 0, 1) * Time.deltaTime;
            }
            if (Input.GetKey(KeyCode.A))
            {
                camera.transform.position -= new Vector3(1, 0, 0) * Time.deltaTime;
            }
            else if (Input.GetKey(KeyCode.D))
            {
                camera.transform.position += new Vector3(1, 0, 0) * Time.deltaTime;
            }
        }
        else
        {
            //Moves the camera back toward the center when too far out
            float diff = Vector2.Distance(cameraCoords, new Vector2(0, 0)) - 9;
            Vector2 dir = cameraCoords.normalized * diff;
            camera.transform.position -= new Vector3(dir.x,0,dir.y)*Time.deltaTime;
        }
        
       
    }
}
