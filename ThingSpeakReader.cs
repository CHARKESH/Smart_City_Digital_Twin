using UnityEngine;
using UnityEngine.Networking;
using System.Collections;
using TMPro;
using System.Text.RegularExpressions;

public class ThingSpeakReader : MonoBehaviour 
{
    [Header("ThingSpeak Settings")]
    public string channelID = "YOUR_CHANNEL_ID";           
    public string readAPIKey = "YOUR_READ_API_KEY"; 
    
    [Header("UI References")]
    public TextMeshProUGUI parkingText;
    public TextMeshProUGUI distanceText;
    public TextMeshProUGUI tempText;
    public TextMeshProUGUI lightText;
    
    [Header("3D Objects")]
    public GameObject gateObject;
    public Light streetLight;
    public GameObject parkingSpot;
    
    private float distance, temp, lightLevel, parkingStatus;
    
    void Start() 
    {
        Debug.Log("ThingSpeakReader Started!");
        StartCoroutine(ReadThingSpeakData());
    }
    
    IEnumerator ReadThingSpeakData() 
    {
        while (true) 
        {
            Debug.Log("Fetching ThingSpeak data...");
            
            string url = $"https://api.thingspeak.com/channels/{channelID}/feeds.json?api_key={readAPIKey}&results=2";
            
            using (UnityWebRequest www = UnityWebRequest.Get(url)) 
            {
                yield return www.SendWebRequest();
                
                if (www.result == UnityWebRequest.Result.Success) 
                {
                    Debug.Log("Data received! Length: " + www.downloadHandler.text.Length);
                    ParseData(www.downloadHandler.text);
                    UpdateVisuals();
                }
                else 
                {
                    Debug.LogError("Error: " + www.error + " | URL: " + url);
                }
            }
            yield return new WaitForSeconds(5f);
        }
    }
    
    void ParseData(string json) 
    {
        Debug.Log("Raw JSON: " + json.Substring(0, Mathf.Min(200, json.Length)));
        
        // Get LAST entry (most recent)
        string lastEntry = ExtractLastEntry(json);
        if (string.IsNullOrEmpty(lastEntry)) 
        {
            Debug.LogError("No valid data entry found!");
            return;
        }
        
        // Parse fields using regex - RELIABLE method
        distance = ExtractFloat(lastEntry, "field1");
        parkingStatus = ExtractFloat(lastEntry, "field2");
        temp = ExtractFloat(lastEntry, "field3");
        lightLevel = ExtractFloat(lastEntry, "field4");
        
        Debug.Log($"Parsed: Dist={distance}, Park={parkingStatus}, Temp={temp}, Light={lightLevel}");
    }
    
    float ExtractFloat(string entry, string fieldName) 
    {
        string pattern = $@"{fieldName}""?:?""?s*([0-9.-]+)";
        Match match = Regex.Match(entry, pattern);
        return match.Success ? float.Parse(match.Groups[1].Value) : 0f;
    }
    
    string ExtractLastEntry(string json) 
    {
        // Find last "feeds" entry - FIXED QUOTES
        int feedsStart = json.IndexOf("\"feeds\":[");
        if (feedsStart == -1) return "";
        
        int lastEntryStart = json.LastIndexOf("{", feedsStart + 10);
        int lastEntryEnd = json.IndexOf("}", lastEntryStart) + 1;
        
        return lastEntryStart > 0 ? json.Substring(lastEntryStart, lastEntryEnd - lastEntryStart) : "";
    }
    
    void UpdateVisuals() 
    {
        // UI Updates
        if (parkingText) parkingText.text = parkingStatus > 0 ? "FULL" : "FREE";
        if (distanceText) distanceText.text = $"Distance: {distance:F1}cm";
        if (tempText) tempText.text = $"Temp: {temp:F1}°C";
        if (lightText) lightText.text = $"Light: {lightLevel:F0}%";
        
        // 3D Updates
        if (gateObject) 
            gateObject.transform.rotation = Quaternion.Euler(0, 
                (distance < 15 && parkingStatus == 0) ? 90 : 0, 0);
                
        if (streetLight) streetLight.intensity = lightLevel / 100f;
        
        if (parkingSpot) 
        {
            Renderer rend = parkingSpot.GetComponent<Renderer>();
            if (rend) rend.material.color = parkingStatus > 0 ? Color.red : Color.green;
        }
        
        Debug.Log("Visuals Updated!");
    }
}
