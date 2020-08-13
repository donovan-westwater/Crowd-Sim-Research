using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TestFunctions : MonoBehaviour
{
    // Start is called before the first frame update
    public GameObject cylinder;
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        if (Input.anyKey) cylinder.SetActive(!cylinder.activeSelf);
    }
}
