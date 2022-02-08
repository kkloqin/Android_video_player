package com.syy.study;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.widget.LinearLayout.LayoutParams;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import androidx.annotation.NonNull;

import com.syy.studio.player.OnInitializedCallback;
import com.syy.studio.player.Player;

public class PlayerActivity extends Activity implements OnSeekBarChangeListener {
	private static final String TAG = "Player_";
	/** GL surface view instance. */
	private SurfaceView surfaceView;
	private Player playerController;
	private SeekBar playerSeekBar;
	private TextView current_time_label;
	private TextView end_time_label;

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_surfaceview_player);

		playerSeekBar = (SeekBar) findViewById(R.id.music_seek_bar);
		playerSeekBar.setOnSeekBarChangeListener(this);
		surfaceView = (SurfaceView) findViewById(R.id.gl_surface_view);
		current_time_label = (TextView) findViewById(R.id.current_time_label);
		end_time_label = (TextView) findViewById(R.id.end_time_label);
		surfaceView.getLayoutParams().height = getWindowManager().getDefaultDisplay().getWidth();
		SurfaceHolder mSurfaceHolder = surfaceView.getHolder();
		mSurfaceHolder.addCallback(previewCallback);
	}

	@Override
	public void onProgressChanged(SeekBar seekBar, int i, boolean b) {

	}

	@Override
	public void onStartTrackingTouch(SeekBar seekBar) {

	}

	@Override
	public void onStopTrackingTouch(SeekBar seekBar) {

	}

	private boolean isFirst = true;
	private SurfaceHolder surfaceHolder = null;
	float totalDuration = 0.0f;
	private Handler handler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {

				default:
					break;
			}
		}

	};

	private Callback previewCallback = new Callback() {
		@Override
		public void surfaceCreated(@NonNull SurfaceHolder holder) {
			surfaceHolder = holder;
			if (isFirst) {
				playerController = new Player() {
					@Override
					public void showLoadingDialog() {
						super.showLoadingDialog();
					}

					@Override
					public void hideLoadingDialog() {
						super.hideLoadingDialog();
					}

					@Override
					public void onCompletion() {
						super.onCompletion();
						playerController.pause();
					}

					@Override
					public void videoDecodeException() {
						super.videoDecodeException();
					}

					@Override
					public void viewStreamMetaCallback(final int width, final int height, float duration) {
						super.viewStreamMetaCallback(width, height, duration);

						PlayerActivity.this.totalDuration = duration;
						handler.post(() -> {
							int screenWidth = getWindowManager().getDefaultDisplay().getWidth();
							int drawHeight = (int) ((float) screenWidth / ((float) width / (float) height));
							LayoutParams params = (LayoutParams) surfaceView.getLayoutParams();
							params.height = drawHeight;
							surfaceView.setLayoutParams(params);

							playerController.resetRenderSize(0, 0, screenWidth, drawHeight);
						});
					}
				};
				playerController.setUseMediaCodec(false);
				int width = getWindowManager().getDefaultDisplay().getWidth();
				String path = Environment.getExternalStorageDirectory() + "/aaa/2.mp4";
				playerController.init(path, holder.getSurface(), width, width, new OnInitializedCallback() {
					public void onInitialized(OnInitialStatus onInitialStatus) {
						Log.i(TAG, "onInitialized called ：" + onInitialStatus);
					}
				});

				isFirst = false;
			} else {
				playerController.onSurfaceCreated(holder.getSurface());
			}

		}

		@Override
		public void surfaceChanged(@NonNull SurfaceHolder surfaceHolder, int i, int i1, int i2) {

		}

		@Override
		public void surfaceDestroyed(@NonNull SurfaceHolder surfaceHolder) {

		}
	};

	@Override
	protected void onDestroy() {
		super.onDestroy();
		// Free the native renderer
		Log.i("problem", "playerController.stop()...");
		playerController.stopPlay();
//		if (null != timerTask) {
//			timerTask.cancel();
//			timerTask = null;
//		}
//		if (null != timer) {
//			timer.cancel();
//			timer = null;
//		}
	}
}
