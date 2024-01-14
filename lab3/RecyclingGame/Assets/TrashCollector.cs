using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
public class TrashCollector : MonoBehaviour
{
    GameObject trash = null;

    public GameObject collectedTrash = null;
    public int score = 0;
    public Text scoreText;
    public Transform hand;
    public GameObject gameOverScreen;
    private void Update()
    {
        if(!collectedTrash && trash && Input.GetButtonDown("Jump"))
        {
            collectedTrash = trash;
            collectedTrash.transform.SetParent(hand);
            collectedTrash.transform.SetLocalPositionAndRotation(Vector3.zero, collectedTrash.transform.rotation);
            collectedTrash.GetComponent<BoxCollider2D>().isTrigger = true;
        }else if(collectedTrash && Input.GetButtonDown("Jump"))
        {
            collectedTrash.transform.SetParent(null);
            collectedTrash.GetComponent<BoxCollider2D>().isTrigger = false;
            collectedTrash = null;
        }


    }

    private void OnTriggerExit2D(Collider2D collision)
    {
        if (trash && trash.name == collision.gameObject.name)
        {
            trash = null;
        }
    }

    private void OnTriggerEnter2D(Collider2D collision)
    {

        if (collision.gameObject.layer == 3)
        {
            trash = collision.gameObject;
            Debug.Log(trash.name);
        }
    }
    public void updateScore()
    {
        scoreText.text = score.ToString();
    }

    public void Die() {

        Destroy(gameObject);
        Time.timeScale = 0;
        gameOverScreen.SetActive(true);

    }
}
