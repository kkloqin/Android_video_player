package com.syy.study;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.syy.study.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'study' library on application startup.
    static {
        System.loadLibrary("study");
    }

    private ActivityMainBinding binding;
    private Button forward_video_player;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());

        forward_video_player = (Button) findViewById(R.id.forward_video_player);
        forward_video_player.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, PlayerActivity.class);
                startActivity(intent);
            }
        });
    }

    /**
     * A native method that is implemented by the 'study' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}