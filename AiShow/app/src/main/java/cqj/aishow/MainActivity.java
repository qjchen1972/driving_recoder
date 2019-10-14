package cqj.aishow;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.StrictMode;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import static android.content.ContentValues.TAG;

public class MainActivity extends AppCompatActivity {

    private static final String LOG_TAG ="aiShow";
    private CameraView textureView_;
    private RelativeLayout overlay_;
    private TextView textView_;
    private String ModelFilePath;

    private String  nativeSampleRate;
    private String  nativeSampleBufSize;
    private boolean supportRecording;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        queryNativeAudioParameters();
        /*if(Build.VERSION.SDK_INT >= 11) {
            StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder().detectDiskReads().detectDiskWrites().detectNetwork().penaltyLog().build());
            StrictMode.setVmPolicy(new StrictMode.VmPolicy.Builder().detectLeakedSqlLiteObjects().detectLeakedClosableObjects().penaltyLog().penaltyDeath().build());
        }*/

        setContentView(R.layout.activity_main);

        overlay_ = (RelativeLayout) findViewById(R.id.overlay);
        textView_ = (TextView) findViewById(R.id.textView);

        textureView_ = (CameraView) findViewById(R.id.texturePreview);


        Button btn_facerec = (Button) findViewById(R.id.btn_facerec);
        //使用匿名内部类
        btn_facerec.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                textureView_.StartCamera();
            }
        });

        Button btn_facestop = (Button) findViewById(R.id.btn_facestop);
        //使用匿名内部类
        btn_facestop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                textureView_.StopCamera();
            }
        });

        Button btn_norm = (Button) findViewById(R.id.btn_norm);
        //使用匿名内部类
        btn_norm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setClass(MainActivity.this, ImageSet.class);
                startActivity(intent);
            }
        });

        Button btn_update = (Button) findViewById(R.id.btn_update);
        //使用匿名内部类
        btn_update.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                for(int i=0;i<ImageSet.faceSet.size();i++){
                    Face p=(Face)ImageSet.faceSet.get(i);
                    int width = p.image.getWidth();
                    int height = p.image.getHeight();
                    int[] imageDate = new int[width*height];
                    p.image.getPixels(imageDate,0,width,0,0,width,height);
                    addtext(jniImpId,p.name+" (" + width + "," + height+" )");
                    addface(jniImpId,p.name,imageDate,width,height);
                }
            }
        });

        jniImpId = createImpId();

        if(mythread_.isstop) {
            mythread_.isstop = false;
            mythread_.start();
        }
        verifyStoragePermissions();


        //new SetUpNeuralNetwork().execute(this);
        //textureView_.init();
        onWindowFocusChanged(true);

        //ModelFilePath = initNetFilePath(this);
        //initNet(jniImpId,ModelFilePath);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        //Log.i("aiShow","onDestroy");
        deleteImpid(jniImpId);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            //| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
        RelativeLayout.LayoutParams overlayParams = (RelativeLayout.LayoutParams) overlay_.getLayoutParams();
        int h = textureView_.getHeight();
        int w = textureView_.getWidth();

        //Log.i(LOG_TAG," it is  "+ h + " ," + w);
        overlayParams.height = textureView_.getHeight() - textureView_.getWidth();
        overlay_.setLayoutParams(overlayParams);
    }

//debug thread
    private MyThread mythread_ = new MyThread();
    public final int MSG_SHOW = 1;
    private final int MSG_MAXLINE = 13;
    Queue<String> queue = new LinkedList<String>();
    private String  debugText="";
    private int  debugline = 0;
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch(msg.what){
                case MSG_SHOW:
                    String  text = getText(jniImpId);
                    if(debugline >= MSG_MAXLINE)
                    {
                        queue.poll();
                        debugline = MSG_MAXLINE;
                    }
                    queue.offer(text);
                    debugText = "";
                    for(String q : queue) {
                        debugText += " "+q +"\n";
                    }
                    debugline++;
                    textView_.setText(debugText);
                    break;
            }
        };
    };
    class MyThread extends Thread {
        public boolean isstop = true;
        @Override
        public void run() {
            while (!isstop) {
                int ret = waitFor(jniImpId,1000);
                if(ret < 0 ) continue;
                //mHandler.sendEmptyMessage(MSG_SHOW);
                Message msg = new Message();
                msg.what = MSG_SHOW;
                mHandler.sendMessage(msg);
            }
        }
    }

//request premission
    private  static final int REQUEST_EXTERNAL_STORAGE = 1;
    private  static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,//"android.permission.READ_EXTERNAL_STORAGE",
            Manifest.permission.WRITE_EXTERNAL_STORAGE,//"android.permission.WRITE_EXTERNAL_STORAGE",
            Manifest.permission.CAMERA,
            Manifest.permission.RECORD_AUDIO,
            Manifest.permission.MODIFY_AUDIO_SETTINGS};

    public  void verifyStoragePermissions() {
        List<String> mPermissionList = new ArrayList<>();
        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(this, PERMISSIONS_STORAGE[1]);

            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                mPermissionList.add(PERMISSIONS_STORAGE[0]);
                mPermissionList.add(PERMISSIONS_STORAGE[1]);
            }

            permission = ActivityCompat.checkSelfPermission(this, PERMISSIONS_STORAGE[2]);
            if (permission != PackageManager.PERMISSION_GRANTED) {
                mPermissionList.add(PERMISSIONS_STORAGE[2]);
            }

           permission = ActivityCompat.checkSelfPermission(this, PERMISSIONS_STORAGE[3]);
            if (permission != PackageManager.PERMISSION_GRANTED) {
                mPermissionList.add(PERMISSIONS_STORAGE[3]);
            }

            if(mPermissionList.isEmpty()) {
                Log.i("aiShow","permission is all ok ");
                textureView_.init();
               if(supportRecording) initAudio(jniImpId,
                        Integer.parseInt(nativeSampleRate),
                        Integer.parseInt(nativeSampleBufSize));
                new SetUpNeuralNetwork().execute(this);
            }
            else{
                Log.i("aiShow","permission request  " + mPermissionList.size());
                String[] permissions = mPermissionList.toArray(new String[mPermissionList.size()]);
                ActivityCompat.requestPermissions(this, permissions, REQUEST_EXTERNAL_STORAGE);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == REQUEST_EXTERNAL_STORAGE){
            for (int i = 0; i < grantResults.length; i++) {
                Log.i("aiShow","result is  " + permissions[i]);
                if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                    //判断是否勾选禁止后不再询问
                    boolean showRequestPermission = ActivityCompat.shouldShowRequestPermissionRationale(MainActivity.this, permissions[i]);
                    if (showRequestPermission) {
                        Log.i("aiShow","no request " + permissions[i]);
                    }
                }
                else{
                    if(permissions[i].equals(PERMISSIONS_STORAGE[1]) ){
                        Log.i("aiShow","init " + permissions[i]);
                        new SetUpNeuralNetwork().execute(this);
                        //ModelFilePath = initNetFilePath(this);
                        //initNet(jniImpId,ModelFilePath);
                    }
                    else if(permissions[i].equals(PERMISSIONS_STORAGE[2])){
                        Log.i("aiShow","init " + permissions[i]);
                        textureView_.init();
                        //ModelFilePath = initNetFilePath(this);
                        //initNet(jniImpId,ModelFilePath);
                    }
                    else if(permissions[i].equals(PERMISSIONS_STORAGE[3])){
                        if(supportRecording) initAudio(jniImpId,
                                Integer.parseInt(nativeSampleRate),
                                Integer.parseInt(nativeSampleBufSize));
                    }
                }
            }
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

//copy file to sd
    private  static final String NETFILEPATH = "/aiShow/";
    private String initNetFilePath(Activity activity) {

        File sdDir = Environment.getExternalStorageDirectory();//获取跟目录
        File file = new File(sdDir.toString() + NETFILEPATH);
        if (!file.exists()) {
            file.mkdir();
        }
        String sdPath = sdDir.toString() + NETFILEPATH;
        //拷贝模型到sk卡
        try {
            copyBigDataToSD(activity, sdPath,"det1.bin");
            copyBigDataToSD(activity, sdPath,"det2.bin");
            copyBigDataToSD(activity, sdPath,"det3.bin");
            copyBigDataToSD(activity, sdPath,"det1.param");
            copyBigDataToSD(activity, sdPath,"det2.param");
            copyBigDataToSD(activity, sdPath,"det3.param");
            copyBigDataToSD(activity, sdPath,"init_net.pb");
            copyBigDataToSD(activity, sdPath,"predict_net.pb");
            copyBigDataToSD(activity, sdPath,"modify_init_net");
            copyBigDataToSD(activity, sdPath,"modify_pred_net");
            copyBigDataToSD(activity, sdPath,"ssd_init_net");
            copyBigDataToSD(activity, sdPath,"ssd_pred_net");

        } catch (IOException e) {
            e.printStackTrace();
            Log.i("aiShow","result is error");
        }
        return sdPath;
    }
    private void copyBigDataToSD(Activity activity, String strDirPath, String strOutFileName) throws IOException {

        String tmpFile = strDirPath + strOutFileName;
        File f = new File(tmpFile);
        if (f.exists()) {
            Log.i("aiShow", "file exists " + strOutFileName);
            return;
        }
        InputStream myInput;
        java.io.OutputStream myOutput = new FileOutputStream(tmpFile);
        myInput = activity.getAssets().open(strOutFileName);
        byte[] buffer = new byte[1024];
        int length = myInput.read(buffer);
        while (length > 0) {
            myOutput.write(buffer, 0, length);
            length = myInput.read(buffer);
        }
        myOutput.flush();
        myInput.close();
        myOutput.close();
        Log.i("aiShow", "end copy file " + strOutFileName);
    }


    private class SetUpNeuralNetwork extends AsyncTask<Activity, Void, Void> {
        @Override
        protected Void doInBackground(Activity... params) {
            Activity act = params[0];
            try {
                ModelFilePath = initNetFilePath(act);
                Log.i("aiShow" , "initNetFilePath " + ModelFilePath);
                initNet(jniImpId,ModelFilePath);
            } catch (Exception e) {
            }
            return null;
        }
    }

    private void queryNativeAudioParameters() {
        supportRecording = true;
        AudioManager myAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        if(myAudioMgr == null) {
            supportRecording = false;
            return;
        }
        nativeSampleRate  =  myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        nativeSampleBufSize =myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);

        // hardcoded channel to mono: both sides -- C++ and Java sides
        int recBufSize = AudioRecord.getMinBufferSize(
                Integer.parseInt(nativeSampleRate),
                AudioFormat.CHANNEL_IN_MONO,
                AudioFormat.ENCODING_PCM_16BIT);
        if (recBufSize == AudioRecord.ERROR ||
                recBufSize == AudioRecord.ERROR_BAD_VALUE) {
            supportRecording = false;
        }
        Log.i("aiShow","rate "+nativeSampleRate +"  size " + nativeSampleBufSize + " buf size " + recBufSize);
    }

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public static long jniImpId = 0;

    private native long createImpId();
    private native void initNet(long impId,String netPath);
    private native String getText(long impId);
    public native static void addtext(long impId,String str);
    private native int waitFor(long impId,int time);
    private native void addface(long impId,String name,int[] imageDate_, int imageWidth, int imageHeight);
    private native void deleteImpid(long impId);
    private native void initAudio(long impId,int sampleRate, int framesPerBuf);
}
