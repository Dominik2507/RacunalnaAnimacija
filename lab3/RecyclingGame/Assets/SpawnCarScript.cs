using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SpawnCarScript : MonoBehaviour
{
    public GameObject carPrefab;
    public MoveDirection moveDirection;
    public int minSpawnRate = 200;
    public int maxSpawnRate = 300;
    int spawnRate;
    int counter = 0;

    private void Start()
    {
        spawnRate = Random.Range(minSpawnRate, maxSpawnRate);
    }
    void FixedUpdate()
    {
        counter++;
        if(counter > spawnRate)
        {
            SpawnCar();
        }
    }

    void SpawnCar()
    {
        GameObject car = Instantiate(carPrefab, gameObject.transform);
        car.GetComponent<CarMove>().moveDirection = moveDirection;
        counter = 0;
        spawnRate = Random.Range(minSpawnRate, maxSpawnRate);
    }
}
