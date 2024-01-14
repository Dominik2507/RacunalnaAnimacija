using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerMovement : MonoBehaviour
{
    public float movementSpeed = 2f;
    public Animator animator;
    public bool lookingLeft = false;
    void Update()
    {
        float x = Input.GetAxis("Horizontal");
        float y = Input.GetAxis("Vertical");

        transform.position += movementSpeed * Time.deltaTime * (new Vector3(x, y));

        animator.SetFloat("playerSpeed", Mathf.Abs(x) + Mathf.Abs(y));

        if(x < -0.01 && !lookingLeft)
        {
            transform.localScale = new Vector3(Mathf.Abs(transform.localScale.x) * -1f, transform.localScale.y, transform.localScale.z );
            lookingLeft = true;
        }else if (x > 0.01 && lookingLeft)
        {
            transform.localScale = new Vector3(Mathf.Abs(transform.localScale.x), transform.localScale.y, transform.localScale.z);
            lookingLeft = false;
        }
    }
}
