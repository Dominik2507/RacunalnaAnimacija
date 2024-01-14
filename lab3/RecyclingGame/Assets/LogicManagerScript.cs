using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;

public class LogicManagerScript : MonoBehaviour
{
    public GameObject menu;
    private bool menuIsOpen = false;

    public AudioSource bgMusic;
    private bool isMuted = false;

    private void Update()
    {
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            if (menuIsOpen) CloseMenu();
            else OpenMenu();
        }
    }
    public void Restart()
    {
        Time.timeScale = 1;
        SceneManager.LoadScene("Level");
    }

    public void OpenMenu()
    {
        Time.timeScale = 0;
        menu.SetActive(true);
        menuIsOpen = true;
        
    }

    public void CloseMenu()
    {
        menuIsOpen = false;
        menu.SetActive(false);
        Time.timeScale = 1;
    }

    public void onExitGame()
    {
        #if UNITY_EDITOR
                UnityEditor.EditorApplication.isPlaying = false;
        #else
                            Application.Quit();
        #endif
    }

    public void toggleMute()
    {
        isMuted = !isMuted;
        bgMusic.mute = isMuted;
    }
}
