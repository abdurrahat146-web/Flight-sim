package com.rahu.flightsim

import android.app.Activity
import android.os.Bundle
import android.view.*
import android.widget.*
import android.graphics.Color

class MainActivity : Activity() {
    init { System.loadLibrary("flightsim") }
    private external fun nativeStart(surface: Surface)
    private external fun nativeStop()
    private external fun nativeControl(x: Float, y: Float, down: Boolean)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN)
        val frame = FrameLayout(this)
        val sim = SurfaceView(this)
        sim.holder.addCallback(object: SurfaceHolder.Callback {
            override fun surfaceCreated(h: SurfaceHolder) { nativeStart(h.surface) }
            override fun surfaceChanged(h: SurfaceHolder, f:Int, w:Int, hgt:Int) {}
            override fun surfaceDestroyed(h: SurfaceHolder) { nativeStop() }
        })
        sim.setOnTouchListener { _, e ->
            nativeControl(e.x / sim.width.coerceAtLeast(1), e.y / sim.height.coerceAtLeast(1), e.action != MotionEvent.ACTION_UP)
            true
        }
        frame.addView(sim)
        val hud = TextView(this).apply {
            text = "FLIGHTSIM PRO\nTouch: pitch / roll / throttle"
            textSize = 16f; setTextColor(Color.WHITE); setPadding(20,18,20,0)
            setShadowLayer(5f,2f,2f,Color.BLACK)
        }
        frame.addView(hud, FrameLayout.LayoutParams(-2,-2))
        setContentView(frame)
    }
    override fun onPause(){ super.onPause(); nativeStop() }
}
