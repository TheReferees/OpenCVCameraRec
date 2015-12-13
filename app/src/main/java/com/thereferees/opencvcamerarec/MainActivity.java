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
import java.util.Arrays;

public class MainActivity extends AppCompatActivity implements CvCameraViewListener2 {

    private static final String TAG = "MainActivity";
    private CameraBridgeViewBase mOpenCvCameraView;

    public native Object[] findObjects(long addrImage);
    public native byte[] testObjects(long addrImage);

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
        //testNDKCode(rgba);
        int[][] objects = (int[][]) findObjects(rgba.getNativeObjAddr());
        int[][] vectors = findVectors(objects, rgba);
        Log.d(TAG, Arrays.deepToString(vectors));
        return rgba;
    }

    //Returns an array of [direction, mass, color] for each object.
    private int[][] findVectors(int[][] objects, Mat rgba) {
        int[][] vectors = new int[objects.length][3];
        for (int i = 0; i < objects.length; i++) {
            double center = ((objects[i][0] + objects[i][1]) / 2.0);
            vectors[i][0] = (int)(center * 180.0 / (rgba.cols())); //X-value
            vectors[i][1] = objects[i][4]; //mass
            vectors[i][2] = objects[i][5]; //colorbits
        }
        return vectors;
    }

    //Tests the code from the NDK which returns an array of pixels.
    //Can be used in conjunction with the testing code.
    private void testNDKCode(Mat rgba) {
        if (counter == 50) {
            byte[] bytes = testObjects(rgba.getNativeObjAddr());
            logToFile(bytes);
        }
        counter++;
    }

    //Creates a new file for logging.
    private void newFile() {
        String fullPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/camerarec";
        try {
            File dir = new File(fullPath);
            if (!dir.exists()) {
                dir.mkdirs();
            }
            OutputStream fOut = null;
            file = new File(fullPath, "log.txt");
            if (file.exists())
                file.delete();
            file.createNewFile();
        } catch (Exception e) {
            Log.e("saveToExternalStorage()", e.getMessage());
        }
    }

    //Logs a string to the log file
    private void logToFile(String string) {
        logToFile(string.getBytes(Charset.defaultCharset()));
    }

    //Logs a byte array to the log file (good for images)
    private void logToFile(byte[] data) {
        newFile();
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
