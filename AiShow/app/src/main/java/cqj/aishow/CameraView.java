package cqj.aishow;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.SurfaceTexture;

import android.media.MediaCodec;

import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;

public class CameraView extends TextureView implements TextureView.SurfaceTextureListener {

    private boolean cameraInitOK = false;
    private boolean windowInitOK = false;
    private Size cameraPreviewSize_;
    //private MediaCodec mediaCodec;

    public CameraView(Context context){
        super(context);
    }



    public void init(){
        createNativeCamera();
        cameraInitOK = true;
        //setSurfaceTextureListener(this);
        //mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar);
    }

    public CameraView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setSurfaceTextureListener(this);
    }
    public void StartCamera() {
        if (isAvailable() && !windowInitOK) {
            onSurfaceTextureAvailable(getSurfaceTexture(),getWidth(), getHeight());
        }
        setPreviewStatus(MainActivity.jniImpId,true);
    }

    public void StopCamera(){
        setPreviewStatus(MainActivity.jniImpId,false);
    }

    private void createNativeCamera() {
        DisplayMetrics dm2 = getResources().getDisplayMetrics();
        int height = dm2.heightPixels;
        int width = dm2.widthPixels;
        initCamera(MainActivity.jniImpId,width, height,true);
        cameraPreviewSize_ = getMinimumCompatiblePreviewSize(MainActivity.jniImpId);
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        //Log.i("aiShow","onSurfaceTextureDestroyed");
        return true;
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface,
                                          int width, int height) {

       //Log.i("aiShow","onSurfaceTextureAvailable  start");
        if(!cameraInitOK) return;
        //Log.i("aiShow","onSurfaceTextureAvailable  w " + cameraPreviewSize_.getWidth() + " h  " + cameraPreviewSize_.getHeight());
        surface.setDefaultBufferSize(cameraPreviewSize_.getWidth(),
                cameraPreviewSize_.getHeight());
        setPreviewWindow(MainActivity.jniImpId, new Surface(surface));
        windowInitOK = true;
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface,
                                            int width, int height) {
    }

    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
    }

    private native void initCamera(long impId,int width, int height,boolean isback);
    private native Size getMinimumCompatiblePreviewSize(long impId);
    private native void setPreviewWindow(long impId, Surface surface);
    private native void setPreviewStatus(long impId, boolean status);


}
