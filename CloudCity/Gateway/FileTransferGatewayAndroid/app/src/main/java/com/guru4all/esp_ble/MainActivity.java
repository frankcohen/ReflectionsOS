/**
// * Transfer files between Seuss display board and Android mobile device using Bluetooth classic
// * This is the ESP32 side of the Gateway, it runs on the Seuss Display Board
// *
// * Board wiring directions and code at https://github.com/frankcohen/ReflectionsOS
// *
// * Reflections project: A wrist watch
// * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
// * player/recorder, SD card, GPS, and accelerometer/compass
// * Repository and community discussions at https://github.com/frankcohen/ReflectionsOS
// * Licensed under GPL v3
// * (c) Frank Cohen, All rights reserved. fcohen@votsh.com
// * Read the license in the license.txt file that comes with this code.

//   Thanks to Manoj Mishra for coding the Bluetooth file transfer protocol on Android and ESP32.
//   Contact Manoj at: Manoj Mishra, Indore, India, http://nevalosstechnologies.com/
*/

package com.guru4all.esp_ble;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothSocket;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.UUID;

import static android.bluetooth.BluetoothProfile.STATE_CONNECTED;

public class MainActivity extends AppCompatActivity {

    private String TAG = "ESP-BT";

    static final int REQUEST_ENABLE_BT = 0;
    static final int CLASSIC_STATE_LISTENING = 1;
    static final int CLASSIC_STATE_CONNECTING = 2;
    static final int CLASSIC_STATE_CONNECTED = 3;
    static final int CLASSIC_STATE_CONNECTION_FAILED = 4;
    static final int CLASSIC_STATE_MESSAGE_RECEIVED = 5;
    static final int CLASSIC_STATE_DISCONNECTED = 6;

    static final int REQUEST_CODE_LOAD_FILE = 100;

    BluetoothManager mbluetoothManager;
    BluetoothAdapter mbluetoothAdapter;
    BluetoothLeScanner mbluetoothLeScanner;

    Switch bluetooth, connect;
    TextView status, filename;
    Button delete, download;
    BluetoothDevice mdevice = null;

    UUID Service_UUID = UUID.fromString("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    UUID Characteristic_UUID_RX = UUID.fromString("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
    UUID Characteristic_UUID_TX = UUID.fromString("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
    UUID Descriptor_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    BluetoothGatt mgatt = null;
    BluetoothGattCharacteristic write_characteristic = null;

    Boolean rcv_file = false;
    Boolean send_file = false;
    Boolean ble_connected = false;
    Client_class client_class = null;
    sendReceive sendReceive = null;

    String file_path;
    File download_file;
    BufferedOutputStream fos = null;

    double expect_bytes = 0, total_bytes = 0;

    SharedPreferences fname;
    LinearLayout flayout;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        status = findViewById(R.id.textView);
        bluetooth = findViewById(R.id.switch1);
        connect = findViewById(R.id.switch2);
        delete = findViewById(R.id.button1);
        download = findViewById(R.id.button2);
        filename = findViewById(R.id.filename);
        flayout = findViewById(R.id.flayout);

        status.setMovementMethod(new ScrollingMovementMethod());
        status.setText("Start\n");

        buttons_enable(false);

        fname = getApplicationContext().getSharedPreferences("filepref", Context.MODE_PRIVATE);
        file_path = fname.getString("filename", "");

        if(file_path.isEmpty()){
            show_toast("Select File First");
            filename.setText("Set File Name");
            buttons_enable(false);
        }
        else {
            download_file = new File(file_path);
            filename.setText("File Name : " + download_file.getName());
        }

        ;

        mbluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mbluetoothAdapter = mbluetoothManager.getAdapter();

        if (isPermissionGranted() && mbluetoothAdapter != null && mbluetoothAdapter.isEnabled()){
            bluetooth.setChecked(true);
            mbluetoothLeScanner = mbluetoothAdapter.getBluetoothLeScanner();
            mbluetoothLeScanner.startScan(mscanCallback);
        }

        bluetooth.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked){
                    if (mbluetoothAdapter == null || !mbluetoothAdapter.isEnabled()){
                        Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                        startActivityForResult(intent, REQUEST_ENABLE_BT);
                    }
                    else {
                        show_toast("Bluetooth Already ON");
                        mbluetoothLeScanner = mbluetoothAdapter.getBluetoothLeScanner();
                        mbluetoothLeScanner.startScan(mscanCallback);
                    }
                }
                else {
                    mdevice = null;
                    if (mbluetoothAdapter.isEnabled()){
                        show_toast("Turning Off Bluetooth");
                        mbluetoothLeScanner.stopScan(mscanCallback);
                        mbluetoothAdapter.disable();
                        mdevice = null;
                        mgatt = null;
                        mbluetoothLeScanner = null;
                        write_characteristic = null;
                        buttons_enable(false);
                        rcv_file = false;
                        send_file = false;
                    }
                    else {
                        show_toast("Bluetooth Already OFF");
                    }
                }
            }
        });

        delete.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

            }
        });

        download.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (download_file != null){
                    try {
                        send_cmd("fn:" + download_file.getName() + " fl:" + download_file.length());
                        Thread.sleep(300);
                        send_cmd("download");
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                else {
                    filename.setText("Select File First");
                    add_status("Select File First");
                }
            }
        });

        flayout.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (ble_connected){
                    Intent intent = new Intent(Intent.ACTION_PICK);
                    intent.setType("*/*");
                    startActivityForResult(intent, REQUEST_CODE_LOAD_FILE);
                }
                else {
                    show_toast("Try Again After Connecting ESP");
                    add_status("Try Again After Connecting ESP");
                }
            }
        });

    }

    void send_cmd(String cmd){
        write_characteristic.setValue(cmd.getBytes());
        mgatt.writeCharacteristic(write_characteristic);
    }

    private void buttons_enable(Boolean en){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (en)
                {
                    delete.setEnabled(true);
                    download.setEnabled(true);
                    delete.getBackground().setColorFilter(null);
                    download.getBackground().setColorFilter(null);
                }
                else {
                    delete.setEnabled(false);
                    download.setEnabled(false);
                    delete.getBackground().setColorFilter(Color.GRAY, PorterDuff.Mode.MULTIPLY);
                    download.getBackground().setColorFilter(Color.GRAY, PorterDuff.Mode.MULTIPLY);
                }
            }
        });
    }

    private ScanCallback mscanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            if (mdevice == null && result.getDevice().getName() != null && result.getDevice().getName().equals("ESP32")){
                add_status("Device : " + result.getDevice().getName());
                add_status("Device Address : " + result.getDevice().getAddress());
                mdevice = mbluetoothAdapter.getRemoteDevice(result.getDevice().getAddress());
                mgatt = mdevice.connectGatt(MainActivity.this, true, mgattCallback);
            }
        }
    };

    private final BluetoothGattCallback mgattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == STATE_CONNECTED) {
                add_status("BLE Connected");
                buttons_enable(true);
                gatt.discoverServices();
                ble_connected = true;
            } else {
                ble_connected = false;
                add_status("BLE Disconnected");
                buttons_enable(false);
                /*mgatt.disconnect();
                mdevice = null;
                Handler han = new Handler(Looper.getMainLooper());
                han.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        mbluetoothManager = (BluetoothManager) getSystemService(BLUETOOTH_SERVICE);
                        mbluetoothAdapter = mbluetoothManager.getAdapter();
                        mbluetoothAdapter.getBluetoothLeScanner().startScan(mscanCallback);
                    }
                }, 2000);*/
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            add_status("Service Discovered");
            write_characteristic = gatt.getService(Service_UUID).getCharacteristic(Characteristic_UUID_RX);
            write_characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
            BluetoothGattCharacteristic mcharacteristic = gatt.getService(Service_UUID).getCharacteristic(Characteristic_UUID_TX);
            gatt.setCharacteristicNotification(mcharacteristic, true);
            BluetoothGattDescriptor descriptor = mcharacteristic.getDescriptor(Descriptor_UUID);
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            gatt.writeDescriptor(descriptor);
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            byte readBuff[] = characteristic.getValue();
            String tmp = new String(readBuff, StandardCharsets.UTF_8);
            if (tmp.contains("fl:")) {
                expect_bytes = Double.parseDouble(tmp.substring(3));
                total_bytes = 0;
                add_status("File Size : " + expect_bytes);
                client_class = new Client_class(mdevice);
                client_class.start();
            } else if (tmp.equals("completed")) {
                if (send_file) {
                    client_class.close();
                    send_file = false;
                    add_status("Data Send Successfully");
                } else {
                    if (total_bytes == expect_bytes) {
                        try {
                            add_status("File Received Completely");
                            fos.close();
                            client_class.close();
                            rcv_file = false;
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    } else {
                        try {
                            add_status("File Not Received Completely");
                            fos.close();
                            download_file.delete();
                            client_class.close();
                            rcv_file = false;
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }
            }else if (tmp.equals("send")) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        download.performClick();
                    }
                });
            }
            else {
                add_status("Value = " + tmp);
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            byte readBuff[] = characteristic.getValue();
            String tmp = new String(readBuff, StandardCharsets.UTF_8);
            if (tmp.equals("download")) {
                send_file = true;
                client_class = new Client_class(mdevice);
                client_class.start();
            }
        }

        @Override
        public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
            add_status("MTU Changed to : " + Integer.toString(mtu));
        }
    };

    private void show_toast(String msg){
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }

    private void add_status(String msg){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                status.append(msg + "\n");
            }
        });
    }

    private byte CRC8(byte arr[], int len){
        byte crc = 0;
        for(int i=0; i<len; i++){
            crc ^= arr[i];
        }
        return crc;
    }

    Handler handler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(@NonNull Message msg) {
            switch (msg.what){
                case CLASSIC_STATE_LISTENING:
                    add_status("LISTENING");
                    break;
                case CLASSIC_STATE_CONNECTING:
                    add_status("CONNECTING");
                    break;
                case CLASSIC_STATE_CONNECTED:
                    add_status("CLASSIC CONNECTED");
                    if (send_file) {
                        try {
                            add_status("SENDING FILE...");
                            long mycount = download_file.length();

                            FileInputStream is = new FileInputStream(download_file);
                            byte[] buffer = new byte[1024];
                            int length;
                            add_status("frankolo2");
                            while ((length = is.read(buffer)) > 0) {
                                sendReceive.write(buffer, 0, length);
                            }
                            add_status("frankolo3");
                        } finally {
                            is.close();
                            sendReceive.close();
                        }
                    }
                    else {
                        send_cmd("start");
                    }
                    break;
                case CLASSIC_STATE_DISCONNECTED:
                    add_status("CLASSIC DISCONNECTED");
                    break;
                case CLASSIC_STATE_CONNECTION_FAILED:
                    add_status("CLASSIC CONNECTION FAILED");
                    break;
                case CLASSIC_STATE_MESSAGE_RECEIVED:
                    byte[] readBuff = (byte[])msg.obj;
                    String tmpMsg = new String(readBuff,0,msg.arg1);
                    status.append(tmpMsg + "\n");
                    break;
            }
            return true;
        }
    });

    private class Client_class extends Thread{
        private BluetoothDevice device;
        private BluetoothSocket socket;
        public Client_class(BluetoothDevice device1){
            device = device1;
            try {
                socket = device.createInsecureRfcommSocketToServiceRecord(MY_UUID);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        public void run(){
            try {
                socket.connect();
                Message message = Message.obtain();
                message.what = CLASSIC_STATE_CONNECTED;
                handler.sendMessage(message);
                sendReceive = new sendReceive(socket);
                sendReceive.start();
            } catch (IOException e) {
                e.printStackTrace();
                Message message = Message.obtain();
                message.what = CLASSIC_STATE_CONNECTION_FAILED;
                handler.sendMessage(message);
            }
        }

        @SuppressLint("SetTextI18n")
        public void close(){
            if(socket.isConnected()){
                try {
                    sendReceive.close();
                    socket.close();
                    Message message = Message.obtain();
                    message.what = CLASSIC_STATE_DISCONNECTED;
                    handler.sendMessage(message);
                } catch (IOException e) {
                    e.printStackTrace();
                    add_status("CANNOT DISCONNECT DEVICE");
                }
            }
        }

        public boolean isConnected(){
            return socket.isConnected();
        }
    }

    private class sendReceive extends Thread{
        private final BluetoothSocket bluetoothSocket;
        private final InputStream inputStream;
        private final OutputStream outputStream;
        private boolean run_state = true;
        private BufferedOutputStream bos = null;

        private sendReceive(BluetoothSocket socket) {
            bluetoothSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                tmpIn = bluetoothSocket.getInputStream();
                tmpOut = bluetoothSocket.getOutputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }

            inputStream = tmpIn;
            outputStream = tmpOut;

            if (rcv_file){
                if (download_file.exists()) download_file.delete();
                if (!download_file.exists()){
                    add_status("File Deleted and Created Empty One");
                    try {
                        download_file.createNewFile();
                        bos = new BufferedOutputStream(new FileOutputStream(download_file));
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        private byte lCRC8(byte arr[], int len){
            byte crc = 0;
            for(int i=0; i<len; i++){
                crc ^= arr[i];
            }
            return crc;
        }

        public void run(){
            byte[] buffer = new byte[1024];
            int bytes;

            while (run_state && bluetoothSocket.isConnected()){
                if (!bluetoothSocket.isConnected()){
                    add_status("CLASSIC DISCONNECTED");
                    run_state = false;
                    if (!client_class.isConnected()){
                        client_class.close();
                        close();
                    }
                }
                try {
                    bytes = inputStream.read(buffer);
                    handler.obtainMessage(CLASSIC_STATE_MESSAGE_RECEIVED, bytes, -1, buffer).sendToTarget();
                } catch (IOException e) {
                    run_state = false;
                    e.printStackTrace();
                }
            }
        }

        public void write(byte[] bytes){
            try {
                outputStream.write(bytes);
                outputStream.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        public boolean isConnected(){
            return run_state;
        }

        public void close(){
            run_state = false;
            try {
                inputStream.close();
                outputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public boolean isPermissionGranted() {
        if (Build.VERSION.SDK_INT >= 23) {
            if (checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION)
                    == PackageManager.PERMISSION_GRANTED && checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    == PackageManager.PERMISSION_GRANTED && checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
                    == PackageManager.PERMISSION_GRANTED) {
                Log.v(TAG,"Permission is granted1");
                return true;
            } else {

                Log.v(TAG,"Permission is revoked1");
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
                return false;
            }
        }
        else {
            Log.v(TAG,"Permission is granted1");
            return true;
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        try {
            mbluetoothLeScanner.stopScan(mscanCallback);
            mbluetoothLeScanner = null;
            if (fos != null){
                fos.close();
            }
            client_class.close();
            rcv_file = false;
            send_file = false;
            mgatt.disconnect();
            finish();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        switch (requestCode){
            case REQUEST_ENABLE_BT:
                if (resultCode == RESULT_OK){
                    show_toast("Bluetooth is On");
                    mbluetoothLeScanner = mbluetoothAdapter.getBluetoothLeScanner();
                    mbluetoothLeScanner.startScan(mscanCallback);
                }
                else {
                    show_toast("Can't Turn on Bluetooth");
                    bluetooth.setChecked(false);
                }
                break;
            case REQUEST_CODE_LOAD_FILE:
                if (resultCode == RESULT_OK){
                    String path = data.getData().getPath();
                    download_file = new File(path);
                    status.append("SENDING FILE : " + download_file.getName() + "\n");
                    status.append("FILE LENGTH : " + download_file.length() + "\n");
                    fname.edit().putString("filename", path).apply();
                    filename.setText("File Name : " + download_file.getName());
                }
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    // Storage Permissions added by frankolo from https://stackoverflow.com/questions/8854359/exception-open-failed-eacces-permission-denied-on-android
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    /**
     * Checks if the app has permission to write to device storage
     *
     * If the app does not has permission then the user will be prompted to grant permissions
     *
     * @param activity
     */
    public static void verifyStoragePermissions(Activity activity) {
        // Check if we have write permission
        int permission = ActivityCompat.checkSelfPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            // We don't have permission so prompt the user
            ActivityCompat.requestPermissions(
                    activity,
                    PERMISSIONS_STORAGE,
                    REQUEST_EXTERNAL_STORAGE
            );
        }
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        int total_result = 0;
        if (requestCode == 1)
        {
            for (int results:grantResults){
                if (grantResults[results] == PackageManager.PERMISSION_GRANTED){
                    Log.v(TAG,"Permission: "+permissions[results]+ " was "+ grantResults[results]);
                    total_result = total_result + grantResults[results];
                }
                else {
                    Log.v(TAG,"Permission: "+permissions[results]+ " was "+ grantResults[results]);
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            show_toast("Required Permissions Not Granted");
                            MainActivity.this.finish();
                        }
                    });
                }
            }

            if (total_result == 0 && mbluetoothAdapter != null && mbluetoothAdapter.isEnabled()){
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        bluetooth.setChecked(true);
                        mbluetoothLeScanner = mbluetoothAdapter.getBluetoothLeScanner();
                        mbluetoothLeScanner.startScan(mscanCallback);
                    }
                });
            }
        }
    }
}
