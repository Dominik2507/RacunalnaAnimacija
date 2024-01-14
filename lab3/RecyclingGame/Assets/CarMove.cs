using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum MoveDirection { left, right}

public class CarMove : MonoBehaviour
{
    public float movementSpeed = 10f;
    public MoveDirection moveDirection;
    public GameObject particleSystem;

    int moveVectorMultiplyer;

    void Start()
    {
        moveVectorMultiplyer = moveDirection == MoveDirection.left ? -1 : 1;
        if (moveDirection == MoveDirection.left)
        {
            transform.position += Vector3.forward;
        }
    }

    void Update()
    {
        transform.position += Vector3.right * movementSpeed * Time.deltaTime * moveVectorMultiplyer;
        if (transform.position.x > 20 || transform.position.x < -20) Destroy(gameObject);
    }

    private void OnTriggerEnter2D(Collider2D collision)
    {
        if (collision.gameObject.CompareTag("Player"))
        {
            collision.gameObject.GetComponent<TrashCollector>().Die();
        }
    }
}
