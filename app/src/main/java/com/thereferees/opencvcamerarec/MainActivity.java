package com.thereferees.opencvcamerarec;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

public class MainActivity extends AppCompatActivity implements CvCameraViewListener2 {

    private static final String TAG = "MainActivity";
    private CameraBridgeViewBase mOpenCvCameraView;

    public native Object[] findObjects(long addrImage);
    public native byte[] detectColors(long addrImage);

    int counter = 0;

    private File file;

    static {
        System.loadLibrary("opencv_java3");
        System.loadLibrary("app");
    }

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    //Log.i(TAG, "OpenCV loaded successfully.");
                    mOpenCvCameraView.enableView();
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //requestWindowFeature(Window.FEATURE_NO_TITLE);
        //getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);
        newFile();
        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.cameraView);
        mOpenCvCameraView.setCvCameraViewListener(this);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mOpenCvCameraView != null) {
            mOpenCvCameraView.disableView();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(OpenCVLoader.initDebug()){
            //Log.d(TAG, "Using OpenCV Library.");
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        } else {
            //Log.d(TAG, "Using OpenCV Manager.");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, mLoaderCallback);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mOpenCvCameraView != null) {
            mOpenCvCameraView.disableView();
        }
    }

    public void onCameraViewStarted (int width, int height){
        //Log.d(TAG, "Yay, the camera started...");
    }

    public void onCameraViewStopped() {
        // Nothing...
    }


    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        Mat rgba = inputFrame.rgba();
        //Imgproc.cvtColor(rgba, rgba, Imgproc.COLOR_BGRA2RGBA, 4);

        Log.d(TAG, rgba.toString());
        //int[][] objects = (int[][]) findObjects(rgba.getNativeObjAddr());
        if (counter == 50) {
            Log.d("MyApp", "ROWS: " + rgba.rows() + ", COLS: " + rgba.cols());
            for (int i = 0; i < 1; i++) {
                for (int j = 0; j < 1; j++) {
                    /*double[] pixel = new double[(int) (rgba.total() * rgba.channels())];
                    rgba.get(0, 0, pixel);*/
                    byte pixel[] = new byte[(int) (rgba.total() * rgba.channels())];
                    rgba.get(j, i, pixel);
                    Log.d(TAG, "STARTING!");
                    //logToFile(pixel);
                    Log.d(TAG, "DONE!");
                    /*for (int k = 0; k < pixel.length; k += rgba.channels()) {
                        String log = "RGB: [";
                        for (int l = 0; l < rgba.channels(); l++) {
                            log += pixel[k + l];
                            log += l == rgba.channels() - 1 ? "" : ", ";
                        }
                        log += "]\n";
                        logToFile(log);

                        Log.d("MyApp", log);
                    }*/
                    //double[] pixel = rgba.get(j, i);
                    //Log.d("MyApp", "" + (i * rgba.cols() + j));
                    //logToFile("" + (i * rgba.cols() + j) + "\n");
                    //logToFile("(" + j + ", " + i + ")" + " " + "RGB: [" + pixel[2] + ", " + pixel[1] + ", " + pixel[0] + ", " + pixel[3] + "]\n");
                    //Log.d("MyApp", "(" + j + ", " + i + ")" + " " + "RGBA: [" + pixel[2] + ", " + pixel[1] + ", " + pixel[0] + ", " + pixel[3] + "]");
                }
            }
            byte[] bytes = detectColors(rgba.getNativeObjAddr());
            Log.d("MyApp", "GAWDDDDD");
            logToFile(bytes);
        } else {
            //Log.d(TAG, "" + counter);
        }
        counter++;
        //Log.d(TAG, Arrays.deepToString(objects));
        return rgba;
    }

    private void newFile() {
        String fullPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/camerarec";
        try {
            File dir = new File(fullPath);
            if (!dir.exists()) {
                dir.mkdirs();
            }
            OutputStream fOut = null;
            file = new File(fullPath, "text.txt");
            if (file.exists())
                file.delete();
            file.createNewFile();
        } catch (Exception e) {
            Log.e("saveToExternalStorage()", e.getMessage());
        }
    }

    private void logToFile(String string) {
        logToFile(string.getBytes(Charset.defaultCharset()));
    }

    private void logToFile(byte[] data) {
        String fullPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/camerarec";
        try
        {
            FileOutputStream fOut = new FileOutputStream(file, true);
            fOut.write(data);
            fOut.flush();
            fOut.close();
        }
        catch (Exception e)
        {
            Log.e("saveToExternalStorage()", e.getMessage());
        }
    }
}
