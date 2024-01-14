using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SpawnTrash : MonoBehaviour
{
    public List<GameObject> trash;
    public Transform trashHolder;
    
    public bool trashSpawned = false;
    public bool willSpawnTrash;

    public float spawnChance = 0.33f;
    private float spawnPositionX;
    void Start()
    {
        willSpawnTrash = Random.Range(0, 1f) <= spawnChance;
        spawnPositionX = Random.Range(-8f, 8f);
    }

    void Update()
    {
        if(willSpawnTrash && !trashSpawned && Mathf.Abs(spawnPositionX - transform.position.x) < 0.1f )
        {
            
            int randomIndex = Random.Range(0, trash.Count);
            GameObject trashPrefab = trash[randomIndex];

            trashSpawned = true;

            Instantiate(trashPrefab, gameObject.transform.position, gameObject.transform.rotation, trashHolder);
        }
        
    }
}
