using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SubmitArea : MonoBehaviour
{
    public string acceptTrash;
    public TrashCollector tc;

    private void OnTriggerEnter2D(Collider2D collision)
    {
        if(collision.gameObject.CompareTag("Player"))
        {
            if (tc.collectedTrash == null) return;

            if (tc.collectedTrash.CompareTag(acceptTrash))
            {
                tc.score++;
                Destroy(tc.collectedTrash);
            }
            else
            {
                tc.score--;
                Destroy(tc.collectedTrash);
            }

            tc.updateScore();
        }
    }

}
