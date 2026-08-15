package com.rahu.flightsim

import android.content.Context
import android.media.MediaPlayer

class AudioManager(private val context: Context) {
    private var engine: MediaPlayer? = null
    private var cabin: MediaPlayer? = null

    fun start() {
        stop()
        engine = MediaPlayer.create(context, R.raw.jet_engine_starting)
        cabin = MediaPlayer.create(context, R.raw.boeing_737_800_air_cabin)
        engine?.isLooping = true
        cabin?.isLooping = true
        cabin?.setVolume(0.35f, 0.35f)
        engine?.start()
        cabin?.start()
    }

    fun stop() {
        engine?.release()
        cabin?.release()
        engine = null
        cabin = null
    }
}
