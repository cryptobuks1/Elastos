package org.elastos.trinity.runtime.notificationmanager;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.widget.Toast;

import org.elastos.trinity.runtime.AppManager;
import org.elastos.trinity.runtime.WebViewActivity;
import org.elastos.trinity.runtime.notificationmanager.db.DatabaseAdapter;

import java.util.ArrayList;
import java.util.HashMap;


public class NotificationManager {
    public static final String LOG_TAG = "NotificationManager";

    private WebViewActivity activity;
    DatabaseAdapter dbAdapter;
    String didSessionDID;

    private static HashMap<String, NotificationManager> instances = new HashMap<>(); // Sandbox DIDs - One did session = one instance

    private ArrayList<NotificationListener> onNotificationListeners = new ArrayList<>();

    public interface NotificationListener {
        void onNotification(Notification notification);
    }

    public NotificationManager(String didSessionDID) {
        this.activity = AppManager.getShareInstance().activity;
        this.dbAdapter = new DatabaseAdapter(this, activity.getBaseContext());
        this.didSessionDID = didSessionDID;

        Log.i(LOG_TAG, "Creating NotificationManager ");
    }

    public static NotificationManager getSharedInstance(String did) {
        if (instances.containsKey(did))
            return instances.get(did);
        else {
            NotificationManager instance = new NotificationManager(did);
            instances.put(did, instance);
            return instance;
        }
    }

    /**
     * Remove an existing notification.
     *
     * @param notificationId Notification ID to remove
     */
    public void clearNotification(String notificationId) {
        dbAdapter.clearNotification(didSessionDID, notificationId);
    }

    /**
     * Get all notifications.
     */
    public ArrayList<Notification> getNotifications() {
        return dbAdapter.getNotifications(didSessionDID);
    }

    /**
     * Sends a notification.
     * @param notificationRequest
     * @param appId
     */
    public void sendNotification(NotificationRequest notificationRequest, String appId) {
        Notification notification = dbAdapter.addNotification(didSessionDID, notificationRequest.key, notificationRequest.title, notificationRequest.message,
                                    notificationRequest.url, notificationRequest.emitter, appId);
        notifyNotification(notification);

        activity.runOnUiThread(() -> Toast.makeText(activity, notificationRequest.title, Toast.LENGTH_SHORT).show());
    }

    /**
     * Registers a listener to know when a notification has been accepted.
     *
     * @param onNotificationListener Called whenever an notification has been sent.
     */
    public void setNotificationListener(NotificationListener onNotificationListener) {
        this.onNotificationListeners.add(onNotificationListener);
    }

    private void notifyNotification(Notification notification) {
        if (onNotificationListeners.size() == 0)
            return;

        if (notification != null) {
            for (NotificationListener listener : onNotificationListeners) {
                listener.onNotification(notification);
            }
        }
    }
}
