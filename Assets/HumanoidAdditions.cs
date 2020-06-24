using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HumanoidAdditions : MonoBehaviour
{
    // Start is called before the first frame update
    public AgentFacing facing;
    public Animator animator;
    Vector3 oldPos;
    void Start()
    {
        oldPos = this.transform.position;
        facing = this.transform.GetComponent<AgentFacing>();
        animator = this.transform.GetComponent<Animator>();
    }

    // Update is called once per frame
    void Update()
    {
        this.transform.eulerAngles = Vector3.RotateTowards(this.transform.eulerAngles.normalized, facing.facing.normalized, 180f, 1f);
        Vector3 speed = this.transform.position - oldPos;
        animator.SetFloat("speed", speed.magnitude);
        oldPos = this.transform.position;
    }
}
