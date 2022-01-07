package com.example.pc_controller;

import androidx.appcompat.app.AppCompatActivity;

import android.util.Log;
import android.os.AsyncTask;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.TextView;
import java.net.URL;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.InputStream;
import java.io.OutputStream;
import android.util.JsonReader;
import org.json.JSONException;
import org.json.JSONObject;
import android.graphics.Color;

public class MainActivity extends AppCompatActivity {

    private Button power;
    private Button reset;
    private Button up;
    private ImageButton refresh;
    private ProgressBar loader;
    private ProgressBar battery;
    private TextView output;
    private TextView batteryLevel;

    private String action = "";
    private String method = "";
    Boolean isUp = false;
    Boolean isFirstTime = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        this.power = findViewById(R.id.power);
        this.reset = findViewById(R.id.reset);
        this.up = findViewById(R.id.up);
        this.refresh = findViewById(R.id.refresh);
        this.loader = findViewById(R.id.loader);
        this.battery = findViewById(R.id.battery);
        this.output = findViewById(R.id.output);
        this.batteryLevel = findViewById(R.id.battery_view);

        power.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                loader.setVisibility(v.VISIBLE);
                output.setText("sending request");
                action = "power";
                SendAPI request = new SendAPI();
                request.execute("POST", action);
            }
        });

        reset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                loader.setVisibility(v.VISIBLE);
                output.setText("sending request");
                action = "reset";
                SendAPI request = new SendAPI();
                request.execute("POST", action);
            }
        });

        refresh.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                loader.setVisibility(v.VISIBLE);
                output.setText("sending request");
                action = "battery_level";
                SendAPI request = new SendAPI();
                request.execute("POST", action);
            }
        });

        up.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                loader.setVisibility(v.VISIBLE);
                output.setText("sending request");
                SendAPI request = new SendAPI();
                request.execute("GET");
            }
        });

        // execute at startup
        method = "GET";
        output.setText("checking PC status");
        SendAPI upRequest = new SendAPI();
        upRequest.execute("GET");
        SharedPreferences sharedPref = getSharedPreferences("data",MODE_PRIVATE);
        int initialBatteryLevel = sharedPref.getInt("battery_level", 50);
        batteryLevel.setText(Integer.toString(initialBatteryLevel) + " %");
        battery.setProgress(initialBatteryLevel);
        action = "battery_level";
        SendAPI initialBatteryRequest = new SendAPI();
        initialBatteryRequest.execute("POST", action);

    }
    class SendAPI extends AsyncTask<String, Integer, Void> {
        String OutputText = "";
        double battery_value = 0;
        @Override
        protected Void doInBackground(String... params) {
            method = params[0];
            String jsonInputString = "{}";
            if(method == "POST") {
                action = params[1];
                try {
                    JSONObject jsonParam = new JSONObject();
                    jsonParam.put("API_key", "a067db7c-bffd-4f26-8151-20a487679dc3");
                    jsonParam.put("action", params[1]);
                    jsonInputString = jsonParam.toString();
                } catch (JSONException e) {
                    Log.d("error", e.getMessage());
                }
            }
            else{
                try {
                    JSONObject jsonParam = new JSONObject();
                    jsonParam.put("API_key", "a067db7c");
                    jsonInputString = jsonParam.toString();
                } catch (JSONException e) {
                    Log.d("error", e.getMessage());
                }
            }
            String error;
            try {
                URL url;
                if(method == "POST"){
                    url = new URL("http://api.snoiry.com/PCcontroller");
                }
                else{
                    url = new URL("http://api.snoiry.com/PCcontroller?API_key=a067db7c");
                }
                try {
                    HttpURLConnection myConnection = (HttpURLConnection) url.openConnection();
                    myConnection.setRequestMethod(method);
                    myConnection.setRequestProperty("User-Agent", "PC controller App v0.1");
                    if(method == "POST"){
                        myConnection.setRequestProperty("Content-Type", "application/json; utf-8");
                        OutputStream os = myConnection.getOutputStream();
                        byte[] input = jsonInputString.getBytes("utf-8");
                        os.write(input, 0, input.length);
                    }
                    if (myConnection.getResponseCode() == 200) {
                        InputStream responseBody = myConnection.getInputStream();
                        InputStreamReader responseBodyReader = new InputStreamReader(responseBody, "UTF-8");
                        JsonReader jsonReader = new JsonReader(responseBodyReader);
                        jsonReader.beginObject(); // Start processing the JSON object

                        Boolean success = false;
                        int status = 0;
                        String response_error = "none";
                        while (jsonReader.hasNext()) { // Loop through all keys
                            String key = jsonReader.nextName(); // Fetch the next key
                            if(key.equals("success")){
                                success = jsonReader.nextBoolean();
                            }
                            else if(key.equals("status")) {
                                status = jsonReader.nextInt();
                            }
                            else if(key.equals("error")){
                                response_error = jsonReader.nextString();
                            }
                            else if(key.equals("value")){
                                battery_value = jsonReader.nextDouble();
                            }
                            else{
                                jsonReader.skipValue(); // Skip values of other keys
                            }
                        }
                        jsonReader.close();

                        if(success == true){
                            if (method == "POST") {
                                if (params[1] == "power") {
                                    OutputText = "PC successfully turned ON";
                                }
                                else if(params[1] == "reset"){
                                    OutputText = "PC successfully reset";
                                }
                                else if (params[1] == "battery_level"){
                                    OutputText = "battery level successfully received";
                                }
                            }
                            else{
                                isUp = true;
                                OutputText = "PC is UP !";
                            }

                        }
                        else{
                            if (method == "GET"){
                                if(status == 200){
                                    isUp = false;
                                    OutputText = "PC is not UP";
                                }
                                else{
                                    OutputText = "error: " + response_error + " (status: " + Integer.toString(status) + ")";
                                }
                            }
                            else {
                                OutputText = "error: " + response_error + " (status: " + Integer.toString(status) + ")";
                            }
                        }

                    } else {
                        OutputText = "error: bad response code";
                    }
                    myConnection.disconnect();
                } catch (IOException ex) {
                    error = ex.getMessage();
                    OutputText = "internal error: " + error;
                }
            } catch (MalformedURLException ex) {
                error = ex.getMessage();
                OutputText = "internal error2: " + error;
            }

            runOnUiThread(new Runnable() {
                public void run() {
                    loader.setVisibility(android.view.View.GONE);
                    String oldText = (String) output.getText();
                    output.setText(OutputText);
                    if (method == "POST") {
                        if(action == "battery_level") {
                            batteryLevel.setText(Double.toString(battery_value) + " %");
                            battery.setProgress( (int) battery_value);
                            SharedPreferences sharedPref = getSharedPreferences("data",MODE_PRIVATE);
                            SharedPreferences.Editor editor = sharedPref.edit();
                            editor.putInt("battery_level", (int) battery_value);
                            editor.commit();
                            if(isFirstTime == true){
                                output.setText(oldText);
                                isFirstTime = false;
                            }
                        }
                    }
                    else{
                        if(isUp == true){
                            up.setTextColor(Color.parseColor("#2ecc71"));
                        }
                        else{
                            up.setTextColor(Color.parseColor("#c0392b"));
                        }
                    }
                }
            });
            return null;
        }

    }
}


