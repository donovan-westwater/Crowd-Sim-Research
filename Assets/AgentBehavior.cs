using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.AI;

public class AgentBehavior : MonoBehaviour
{
    [SerializeField] Transform target;
    // Start is called before the first frame update
    void Start()
    {
        this.gameObject.GetComponent<NavMeshAgent>().destination = target.position;
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
