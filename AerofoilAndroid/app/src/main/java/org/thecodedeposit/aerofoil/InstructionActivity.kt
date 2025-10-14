package org.thecodedeposit.aerofoil

import android.app.Activity
import android.os.Bundle
import android.view.KeyEvent
import android.view.MotionEvent

class InstructionActivity : Activity() {

    private val layouts = listOf(
        R.layout.instruct_1,
        R.layout.instruct_2,
        R.layout.instruct_3,
        R.layout.instruct_4,
        R.layout.gamepad
    )
    private var currentIndex = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(layouts[currentIndex])
    }

    private fun showNextInstruction() {
        currentIndex++
        if (currentIndex < layouts.size) {
            setContentView(layouts[currentIndex])
        } else {
            finish()
        }
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        if (event?.action == MotionEvent.ACTION_UP) {
            showNextInstruction()
        }
        return true
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {
        showNextInstruction()
        return true
    }
}