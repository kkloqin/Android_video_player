package com.syy.study;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.View;
import android.widget.LinearLayout.LayoutParams;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;

import com.syy.studio.player.OnInitializedCallback;
import com.syy.studio.player.Player;

import java.util.Timer;
import java.util.TimerTask;

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
		findViewById(R.id.pause_btn).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				playerController.pause();
			}
		});
		findViewById(R.id.continue_btn).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				timer = new Timer();
				timerTask = new TimerTask() {
					@Override
					public void run() {
						playTimeSeconds = playerController.getPlayProgress();
						bufferedTimeSeconds = playerController.getBufferedProgress();
						handler.sendEmptyMessage(UPDATE_PLAY_VIEDO_TIME_FLAG);
					}
				};
				timer.schedule(timerTask, 500, 100);

				playerController.play();
				timerStart();
			}
		});
		timerStart();
	}

	protected void timerStop() {
		timerTask.cancel();
		timer.cancel();
	}

	protected void timerStart() {
		timer = new Timer();
		timerTask = new TimerTask() {
			@Override
			public void run() {
				if(null != playerController) {
					playTimeSeconds = playerController.getPlayProgress();
					bufferedTimeSeconds = playerController.getBufferedProgress();
					handler.sendEmptyMessage(UPDATE_PLAY_VIEDO_TIME_FLAG);
				}
			}
		};
		timer.schedule(timerTask, 500, 100);
	}

	float playTimeSeconds = 0.0f;
	float bufferedTimeSeconds = 0.0f;
	float totalDuration = 0.0f;
	public static final int UPDATE_PLAY_VIEDO_TIME_FLAG = 1201;
	Timer timer;
	TimerTask timerTask;
	SurfaceHolder surfaceHolder = null;

	@Override
	protected void onDestroy() {
		super.onDestroy();
		// Free the native renderer
		Log.i("problem", "playerController.stop()...");
		playerController.stopPlay();
		if (null != timerTask) {
			timerTask.cancel();
			timerTask = null;
		}
		if (null != timer) {
			timer.cancel();
			timer = null;
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		switch (keyCode) {
			case KeyEvent.KEYCODE_BACK:
				finish();
				return true;
		}
		return super.onKeyDown(keyCode, event);
	}

	private static final int PLAY_END_FLAG = 12330;
	private Handler handler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
				case UPDATE_PLAY_VIEDO_TIME_FLAG:
					if (!isDragging) {
						String curtime = String.format("%02d:%02d", (int) playTimeSeconds / 60, (int) playTimeSeconds % 60);
						String totalTime = String.format("%02d:%02d", (int) totalDuration / 60, (int) totalDuration % 60);
						current_time_label.setText(curtime);
						end_time_label.setText(totalTime);
						int progress = totalDuration == 0.0f ? 0 : (int) (playTimeSeconds * 100 / totalDuration);
						int secondProgress = totalDuration == 0.0f ? 0 : (int) (bufferedTimeSeconds * 100 / totalDuration);
						playerSeekBar.setProgress(progress);
						playerSeekBar.setSecondaryProgress(secondProgress);
					}
					break;
				case PLAY_END_FLAG:
					Toast.makeText(PlayerActivity.this, "??????????????????", Toast.LENGTH_SHORT).show();
					break;
				default:
					break;
			}
		}

	};

	boolean isFirst = true;

	private Callback previewCallback = new Callback() {

		public void surfaceCreated(SurfaceHolder holder) {
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
						timerTask.cancel();
						timerTask = null;
						timer.cancel();
						timer = null;
						playerSeekBar.setProgress(0);
						playerSeekBar.setSecondaryProgress(0);
						playerController.seekToPosition(0);
					}

					@Override
					public void videoDecodeException() {
						super.videoDecodeException();
					}

					@Override
					public void viewStreamMetaCallback(final int width, final int height, float duration) {
						super.viewStreamMetaCallback(width, height, duration);

						PlayerActivity.this.totalDuration = duration;
						handler.post(new Runnable() {

							@Override
							public void run() {
								int screenWidth = getWindowManager().getDefaultDisplay().getWidth();
								int drawHeight = (int) ((float) screenWidth / ((float) width / (float) height));
								LayoutParams params = (LayoutParams) surfaceView.getLayoutParams();
								params.height = drawHeight;
								surfaceView.setLayoutParams(params);

								playerController.resetRenderSize(0, 0, screenWidth, drawHeight);
							}
						});
					}

				};
				playerController.setUseMediaCodec(false);
				int width = getWindowManager().getDefaultDisplay().getWidth();
				String path = Environment.getExternalStorageDirectory() + "/aaa/2.mp4";
				Log.d("problem", "surfaceCreated: "+ path);
				playerController.init(path, holder.getSurface(), width, width, new OnInitializedCallback() {
					public void onInitialized(OnInitialStatus onInitialStatus) {
						// TODO: do your work here
						Log.i("problem", "onInitialized called");
					}
				});

				isFirst = false;
			}

			else {
				playerController.onSurfaceCreated(holder.getSurface());
			}
		}

		public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
			playerController.resetRenderSize(0, 0, width, height);
		}

		public void surfaceDestroyed(SurfaceHolder holder) {
			playerController.onSurfaceDestroyed(holder.getSurface());
		}
	};

	/** ??????????????????????????????????????????????????????????????????seekbar????????? **/
	private boolean isDragging = false;

	@Override
	public void onStartTrackingTouch(SeekBar seekBar) {
		isDragging = true;
		playerController.beforeSeekCurrent();
		Log.i("problem", "onStartTrackingTouch");
	}

	@Override
	public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		if (isDragging) {
			float pro = seekBar.getProgress();
			float num = seekBar.getMax();
			float result = pro / num;

			seekCurrent(result * totalDuration);

//			Log.i("problem", "onProgressChanged "+result * totalDuration);
		}
		else {
			// ????????????seek?????????????????????????????????????????????isDragging??????false????????????????????????????????????????????????
//			Log.i("problem", "this change from play callback "+progress);
		}
	}

	@Override
	public void onStopTrackingTouch(SeekBar seekBar) {
		isDragging = false;
		float pro = seekBar.getProgress();
		float num = seekBar.getMax();
		float result = pro / num;
		seekToPosition(result * totalDuration);

		playerController.afterSeekCurrent();

		Log.i("problem", "onStopTrackingTouch");
	}

	public void seekToPosition(float position) {
		Log.i("problem", "position:" + position);
		playerController.seekToPosition(position);
	}

	public void seekCurrent(float position) {
//		Log.i("problem", "position:" + position);
		playerController.seekCurrent(position);
	}
}
