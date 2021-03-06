package org.elastos.trinity.runtime.contactnotifier;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.util.Log;

import org.elastos.carrier.Carrier;
import org.elastos.carrier.ConnectionStatus;
import org.elastos.carrier.FriendInfo;
import org.elastos.carrier.PresenceStatus;
import org.elastos.carrier.exceptions.CarrierException;
import org.elastos.did.DID;
import org.elastos.did.DIDDocument;
import org.elastos.did.VerifiableCredential;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDResolveException;
import org.elastos.trinity.plugins.did.DIDPlugin;
import org.elastos.trinity.runtime.R;
import org.elastos.trinity.runtime.contactnotifier.comm.CarrierHelper;
import org.elastos.trinity.runtime.contactnotifier.db.DatabaseAdapter;
import org.elastos.trinity.runtime.contactnotifier.db.ReceivedInvitation;
import org.elastos.trinity.runtime.contactnotifier.db.SentInvitation;
import org.elastos.trinity.runtime.notificationmanager.NotificationManager;
import org.elastos.trinity.runtime.notificationmanager.NotificationRequest;

import java.util.ArrayList;
import java.util.HashMap;

public class ContactNotifier {
    public static final String LOG_TAG = "ContactNotifier";

    private static final String ONLINE_STATUS_MODE_PREF_KEY = "onlinestatusmode";
    private static final String INVITATION_REQUESTS_MODE_PREF_KEY = "invitationrequestsmode";

    private static final String FRIENDS_APP_PACKAGE_ID = "org.elastos.trinity.dapp.friends";

    private static HashMap<String, ContactNotifier> instances = new HashMap<>(); // Sandbox DIDs - One did session = one instance

    private Context context;
    String didSessionDID;
    DatabaseAdapter dbAdapter;
    CarrierHelper carrierHelper;

    private ArrayList<OnOnlineStatusListener> onOnlineStatusChangedListeners = new ArrayList<>();
    private ArrayList<OnInvitationAcceptedByFriendListener> onInvitationAcceptedListeners = new ArrayList<>();

    public interface OnInvitationAcceptedByFriendListener {
        void onInvitationAccepted(Contact contact);
    }

    public interface OnInvitationAcceptedByUsListener {
        void onInvitationAccepted(Contact contact);
        void onNotExistingInvitation();
        void onError(String reason);
    }

    public interface OnOnlineStatusListener {
        void onStatusChanged(Contact contact, OnlineStatus status);
    }

    private ContactNotifier(Context context, String didSessionDID) throws Exception {
        // Initialize the DID back end as we will need it. Though it's not clean to call the DID plugin
        // Directly for this. Need to create a DID class inside runtime.
        try {
            DIDPlugin.initializeDIDBackend(context);
        }
        catch (DIDResolveException e) {
            e.printStackTrace(); // Will not happen - invalid exception
        }
        catch (Exception e) {
            e.printStackTrace();
            Log.e(LOG_TAG, "Unable to initialize the DID backend. Contact notifier will not work correctly");
        }

        this.context = context;
        this.didSessionDID = didSessionDID;
        this.dbAdapter = new DatabaseAdapter(this, context);
        this.carrierHelper = new CarrierHelper(this, didSessionDID, context);

        Log.i(LOG_TAG, "Creating contact notifier instance for DID session "+didSessionDID);

        listenToCarrierHelperEvents();
    }

    public static ContactNotifier getSharedInstance(Context context, String did) throws Exception {
        if (instances.containsKey(did))
            return instances.get(did);
        else {
            ContactNotifier instance = new ContactNotifier(context, did);
            instances.put(did, instance);
            return instance;
        }
    }

    /**
     * Returns DID-session specific carrier address for the current user. This is the address
     * that can be shared with future contacts so they can send invitation requests.
     *
     * @returns The currently active carrier address on which user can be reached by (future) contacts.
     */
    public String getCarrierAddress() throws CarrierException {
        return carrierHelper.getOrCreateAddress();
    }

    /**
     * Retrieve a previously added contact from his DID.
     *
     * @param did The contact's DID.
     */
    public Contact resolveContact(String did) {
        if (did == null)
            return null;

        return dbAdapter.getContactByDID(didSessionDID, did);
    }

    /**
     * Returns the list of all contacts.
     */
    public ArrayList<Contact> getAllContacts() {
        return dbAdapter.getAllContacts(didSessionDID);
    }

    /**
     * Remove an existing contact. This contact stops seeing user's online status, can't send notification
     * any more.
     *
     * @param did DID of the contact to remove
     */
    public void removeContact(String did) throws Exception {
        Contact contact = resolveContact(did);
        if (contact == null) {
            throw new Exception("No contact found with DID "+did);
        }

        // Remove from carrier
        carrierHelper.removeFriend(did, contact.carrierUserID, (succeeded, reason)->{
            // Remove from database
            dbAdapter.removeContact(didSessionDID, did);
        });
    }

    /**
     * Listen to changes in contacts online status.
     *
     * @param onOnlineStatusChanged Called every time a contact becomes online or offline.
     */
    public void addOnlineStatusListener(OnOnlineStatusListener onOnlineStatusChanged) {
        this.onOnlineStatusChangedListeners.add(onOnlineStatusChanged);
    }

    /**
     * Changes the online status mode, that decides if user's contacts can see his online status or not.
     *
     * @param onlineStatusMode Whether contacts can see user's online status or not.
     */
    public void setOnlineStatusMode(OnlineStatusMode onlineStatusMode) {
        getPrefs().edit().putInt(ONLINE_STATUS_MODE_PREF_KEY, onlineStatusMode.mValue).apply();
        carrierHelper.setOnlineStatusMode(onlineStatusMode);
    }

    /**
     * Returns the current online status mode.
     */
    public OnlineStatusMode getOnlineStatusMode() {
        int onlineStatusModeAsInt = getPrefs().getInt(ONLINE_STATUS_MODE_PREF_KEY, OnlineStatusMode.STATUS_IS_VISIBLE.mValue);
        return OnlineStatusMode.fromValue(onlineStatusModeAsInt);
    }

    /**
     * Sends a contact request to a peer. This contact will receive a notification about this request
     * and can choose to accept the invitation or not.
     *
     * In case the invitation is accepted, both peers become friends on carrier and in this contact notifier and can
     * start sending remote notifications to each other.
     *
     * Use invitation accepted listener API to get informed of changes.
     *
     * @param carrierAddress Target carrier address. Usually shared privately or publicly by the future contact.
     */
    public void sendInvitation(String targetDID, String carrierAddress) throws Exception {
        // It's possible that we are already connected to that friend, for example if we added him earlier then
        // deleted him. In such case, we don't send any invitation and we directly re-add the friend as a contact.
        FriendInfo existingCarrierFriend = carrierHelper.getFriendUserInfoFromAddress(carrierAddress);
        if (existingCarrierFriend != null) {
            Log.d(LOG_TAG, "Already a carrier friend.");
            // Try to find more info about this contact
            resolveOnChainDIDInfo(targetDID, (name, avatarHash)->{
                Log.d(LOG_TAG, "Already a carrier friend. Directly adding as a new contact locally");
                Contact addedContact = dbAdapter.addContact(didSessionDID, targetDID, existingCarrierFriend.getUserId());
                if (addedContact != null) {
                    if (name != null) {
                        // Save contact name to database for better display later on
                        addedContact.setName(name);
                    }
                }
            });
        }
        else {
            Log.d(LOG_TAG, "Not yet a carrier friend. Sending a carrier invitation.");
            carrierHelper.sendInvitation(carrierAddress, (succeeded, reason)->{
                if (succeeded) {
                    dbAdapter.addSentInvitation(didSessionDID, targetDID, carrierAddress);
                }
            });
        }
    }

    /**
     * Accepts an invitation sent by a remote peer. After accepting an invitation, a new contact is saved
     * with his did and carrier addresses. After that, this contact can be resolved as a contact object
     * from his did string.
     *
     * @param invitationId Received invitation id (database) that we're answering for.
     */
    public void acceptInvitation(String invitationId, OnInvitationAcceptedByUsListener listener) {
        // Retrieved the received invitation info from a given ID
        ReceivedInvitation invitation = dbAdapter.getReceivedInvitationById(didSessionDID, invitationId);
        if (invitation == null) {
            // No such invitation exists.
            listener.onNotExistingInvitation();
            return;
        }

        // Accept the invitation on carrier
        try {
            carrierHelper.acceptFriend(invitation.carrierUserID, (succeeded, reason)->{
                if (succeeded) {
                    // Add the contact to our database
                    Log.d(LOG_TAG, "Accepting a friend invitation. Adding contact locally");
                    Contact contact = dbAdapter.addContact(didSessionDID, invitation.did, invitation.carrierUserID);

                    // Delete the pending invitation request
                    dbAdapter.removeReceivedInvitation(didSessionDID, invitationId);

                    listener.onInvitationAccepted(contact);
                }
                else {
                    listener.onError(reason);
                }
            });
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Rejects an invitation sent by a remote peer. This inviation is permanently deleted.
     * The invitation is rejected only locally. The sender is not aware of it.
     *
     * @param invitationId Received invitation id.
     */
    public void rejectInvitation(String invitationId) {
        // Retrieved the received invitation info from a given ID
        ReceivedInvitation invitation = dbAdapter.getReceivedInvitationById(didSessionDID, invitationId);
        if (invitation == null) {
            // No such invitation exists.
            return;
        }

        try {
            // Delete the invitation
            dbAdapter.removeReceivedInvitation(didSessionDID, invitationId);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Registers a listener to know when a previously sent invitation has been accepted.
     * Currently, it's only possible to know when an invitation was accepted, but not when
     * it was rejected.
     *
     * @param onInvitationAcceptedListener Called whenever an invitation has been accepted.
     */
    public void addOnInvitationAcceptedListener(OnInvitationAcceptedByFriendListener onInvitationAcceptedListener) {
        this.onInvitationAcceptedListeners.add(onInvitationAcceptedListener);
    }

    private void notifyInvitationAcceptedByFriend(Contact contact) {
        if (onInvitationAcceptedListeners.size() == 0)
            return;

        if (contact != null) {
            for (OnInvitationAcceptedByFriendListener listener : onInvitationAcceptedListeners) {
                listener.onInvitationAccepted(contact);
            }
        }
    }

    /**
     * Configures the way invitations are accepted: manually, or automatically.
     *
     * @param mode Whether invitations should be accepted manually or automatically.
     */
    public void setInvitationRequestsMode(InvitationRequestsMode mode) {
        getPrefs().edit().putInt(INVITATION_REQUESTS_MODE_PREF_KEY, mode.mValue).apply();
    }

    /**
     * Returns the way invitations are accepted.
     */
    public InvitationRequestsMode getInvitationRequestsMode() {
        int invitationRequestsModeAsInt = getPrefs().getInt(INVITATION_REQUESTS_MODE_PREF_KEY, InvitationRequestsMode.AUTO_ACCEPT.mValue);
        return InvitationRequestsMode.fromValue(invitationRequestsModeAsInt);
    }

    /**
     * DID Session sandboxed preferences
     */
    public SharedPreferences getPrefs() {
        return context.getSharedPreferences("CONTACT_NOTIFIER_PREFS_"+didSessionDID, Context.MODE_PRIVATE);
    }

    private void listenToCarrierHelperEvents() {
        carrierHelper.setCarrierEventListener(new CarrierHelper.OnCarrierEventListener() {
            @Override
            public void onFriendRequest(String did, String carrierUserId) {
                // Received an invitation from a potential contact.

                // If friend acceptation mode is set to automatic, we directly accept this invitation.
                // Otherwise, we let the contact notifier know this and it will send a notification to user.
                if (getInvitationRequestsMode() == InvitationRequestsMode.AUTO_ACCEPT) {
                    Log.i(ContactNotifier.LOG_TAG, "Auto-accepting friend invitation");

                    try {
                        carrierHelper.acceptFriend(carrierUserId, (succeeded, reason)->{
                            if (succeeded) {
                                // Try to find more info about this contact
                                resolveOnChainDIDInfo(did, (name, avatarHash)->{
                                    String targetUrl = "https://scheme.elastos.org/viewfriend?did="+did;

                                    boolean notificationSent = false;
                                    Log.d(LOG_TAG, "Adding contact locally");
                                    Contact addedContact = dbAdapter.addContact(didSessionDID, did, carrierUserId);
                                    if (addedContact != null) {
                                        if (name != null) {
                                            // Save contact name to database for better display later on
                                            addedContact.setName(name);
                                            sendLocalNotification(did,"newcontact-"+did, context.getString(R.string.notification_contact_added), name + context.getString(R.string.notification_name_was_added_new), targetUrl, FRIENDS_APP_PACKAGE_ID);
                                            notificationSent = true;
                                        }
                                    }

                                    if (!notificationSent) {
                                        sendLocalNotification(did,"newcontact-"+did, context.getString(R.string.notification_contact_added), context.getString(R.string.notification_someone_was_added_new), targetUrl, FRIENDS_APP_PACKAGE_ID);
                                    }
                                });
                            }
                        });
                    }
                    catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                else if (getInvitationRequestsMode() == InvitationRequestsMode.AUTO_REJECT) {
                    // Just forget this request, as user doesn't want to be bothered.
                }
                else {
                    // MANUALLY_ACCEPT - Manual approval
                    long invitationID = dbAdapter.addReceivedInvitation(didSessionDID, did, carrierUserId);

                    // Try to find more info about this contact
                    resolveOnChainDIDInfo(did, (name, avatarHash)-> {
                        String targetUrl = "https://scheme.elastos.org/viewfriendinvitation?did="+did+"&invitationid="+invitationID;

                        if (name != null)
                            sendLocalNotification(did,"contactreq-"+did, context.getString(R.string.notification_contact_request), name + context.getString(R.string.notification_name_want_add), targetUrl, FRIENDS_APP_PACKAGE_ID);
                        else
                            sendLocalNotification(did,"contactreq-"+did, context.getString(R.string.notification_contact_request), context.getString(R.string.notification_someone_want_add), targetUrl, FRIENDS_APP_PACKAGE_ID);
                    });
                }
            }

            @Override
            public void onFriendOnlineStatusChange(FriendInfo info) {
                updateFriendOnlineStatus(info);
            }

            @Override
            public void onFriendPresenceStatusChange(FriendInfo info) {
                updateFriendOnlineStatus(info);
            }

            @Override
            public void onRemoteNotification(String friendId, RemoteNotificationRequest remoteNotification) {
                // Try to resolve this friend id as a contact
                Contact contact = dbAdapter.getContactByCarrierUserID(didSessionDID, friendId);
                if (contact != null) {
                    // Make sure this contact is not blocked by us
                    if (!contact.notificationsBlocked) {
                        sendLocalNotification(contact.did,remoteNotification.key, remoteNotification.title, remoteNotification.message, remoteNotification.url);
                    }
                    else {
                        Log.w(ContactNotifier.LOG_TAG, "Not delivering remote notification because contact is blocked");
                    }
                }
                else {
                    Log.w(ContactNotifier.LOG_TAG, "Remote notification received from unknown contact. Dropping. Friend ID = "+friendId);
                }
            }

            @Override
            public void onFriendAdded(FriendInfo info) {
                checkFriendInvitationAccepted(info.getUserId());
            }
        });
    }

    private void updateFriendOnlineStatus(FriendInfo info) {
        // Resolve the contact and make sure this friend wants to be seen.
        Contact contact = dbAdapter.getContactByCarrierUserID(didSessionDID, info.getUserId());
        if (contact != null) {
            if (info.getPresence() == PresenceStatus.None) {
                notifyOnlineStatusChanged(info.getUserId(), info.getConnectionStatus());
            }
            else {
                // User doesn't want to be seen
                notifyOnlineStatusChanged(info.getUserId(), ConnectionStatus.Disconnected);
            }
        }
        else {
            // If we receive an online status information from a friend but this friend is not in our contact list yet,
            // AND this friend is in our sent invitations list, this means the friend has accepted our previous invitation.
            // This is the only way to get this information from carrier. So in such case, we can add hims as a real contact
            // now, and remove the sent invitation.
            checkFriendInvitationAccepted(info.getUserId());
        }
    }

    private void checkFriendInvitationAccepted(String carrierUserId) {
        Log.d(LOG_TAG, "Checking if a carrier peer id should be added as a friend");
        SentInvitation invitation = findSentInvitationByFriendId(carrierUserId);
        if (invitation != null) {
            Log.d(LOG_TAG, "There is an on going invitation. Adding friend");
            handleFriendInvitationAccepted(invitation, carrierUserId);
        }
        else {
            Log.d(LOG_TAG, "There is no on going invitation.");
        }
    }

    /**
     * When a friend accepts our invitation, the only way to know it is to match all friends userIds with our pending
     * invitation carrier addresses manually. Not convenient, but that's the only way for now.
     */
    private SentInvitation findSentInvitationByFriendId(String friendId) {
        ArrayList<SentInvitation> invitations = dbAdapter.getAllSentInvitations(didSessionDID);
        for (SentInvitation invitation : invitations) {
            if (invitation.carrierAddress != null) {
                // Resolve user id associated with the invitation carrier address to be able to compare it
                String invitationUserID = Carrier.getIdFromAddress(invitation.carrierAddress);
                if (invitationUserID != null && invitationUserID.equals(friendId)) {
                    // We found a pending invitation that matches the given friend.
                    return invitation;
                }
            }
        }
        return null;
    }

    /**
     * A potential friend to whom we've sent an invitation earlier has accepted it. So we can now consider it as
     * a "contact".
     */
    private void handleFriendInvitationAccepted(SentInvitation invitation, String friendId) {
        Log.d(LOG_TAG, "Friend has accepted our invitation. Adding contact locally");

        // Add carrier friend as a contact
        Contact addedContact = dbAdapter.addContact(didSessionDID, invitation.did, friendId);

        // Delete the pending invitation request
        dbAdapter.removeSentInvitationByAddress(didSessionDID, invitation.carrierAddress);

        resolveOnChainDIDInfo(invitation.did, (name, avatarHash)-> {
            String targetUrl = "https://scheme.elastos.org/viewfriend?did="+invitation.did;

            if (name != null) {
                // Save name to database for later use
                addedContact.setName(name);
                sendLocalNotification(invitation.did,"friendaccepted-"+invitation.did, context.getString(R.string.notification_contact_invitation_accepted),name + context.getString(R.string.notification_name_accept_invitation), targetUrl, FRIENDS_APP_PACKAGE_ID);
            }
            else {
                sendLocalNotification(invitation.did,"friendaccepted-"+invitation.did, context.getString(R.string.notification_contact_invitation_accepted), context.getString(R.string.notification_someone_accept_invitation), targetUrl, FRIENDS_APP_PACKAGE_ID);
            }

            // Notify the listeners
            notifyInvitationAcceptedByFriend(addedContact);
        });
    }

    private void notifyOnlineStatusChanged(String friendId, ConnectionStatus status) {
        if (onOnlineStatusChangedListeners.size() == 0)
            return;

        // Resolve contact from friend ID
        Contact contact = dbAdapter.getContactByCarrierUserID(didSessionDID, friendId);
        if (contact != null) {
            for (OnOnlineStatusListener listener : onOnlineStatusChangedListeners) {
                listener.onStatusChanged(contact, onlineStatusFromCarrierStatus(status));
            }
        }
    }

    public OnlineStatus onlineStatusFromCarrierStatus(ConnectionStatus status) {
        switch (status) {
            case Connected:
                return OnlineStatus.ONLINE;
            case Disconnected:
                return OnlineStatus.OFFLINE;
        }

        // No clean info - considered as offline.
        return OnlineStatus.OFFLINE;
    }

    /**
     * NOTE: As carrier can't really hide user's visibility from the user side, we use the "presence status" information
     * to let friends plugins know whether user wants to show his presence or not. This is not a ready away or online status.
     */
    public PresenceStatus onlineStatusModeToPresenceStatus(OnlineStatusMode status) {
        switch (status) {
            case STATUS_IS_VISIBLE:
                return PresenceStatus.None;
            case STATUS_IS_HIDDEN:
                return PresenceStatus.Away;
        }

        // No clean info - considered as hidden.
        return PresenceStatus.Away;
    }

    void sendLocalNotification(String relatedRemoteDID, String key, String title, String message, String url) {
        sendLocalNotification(relatedRemoteDID, key, title, message, url, "system");
    }

    void sendLocalNotification(String relatedRemoteDID, String key, String title, String message, String url, String appId) {
        NotificationRequest notification = new NotificationRequest();
        notification.key = key;
        notification.title = title;
        notification.message = message;
        notification.emitter = relatedRemoteDID;
        notification.url = url;
        try {
            // NOTE: appid can't be null because the notification manager uses it for several things.
            NotificationManager.getSharedInstance(this.didSessionDID).sendNotification(notification, appId);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Retrieves info about a given remote DID from its did document (on chain).
     */
    @SuppressLint("StaticFieldLeak")
    void resolveOnChainDIDInfo(String did, OnChainDIDInfoReceived listener) {
        Log.d(LOG_TAG, "Trying to get more info about did "+did+" on chain.");
        new AsyncTask<Void, Void, DIDDocument>() {
            @Override
            protected DIDDocument doInBackground(Void... voids) {
                DIDDocument didDocument;
                try {
                    didDocument = new DID(did).resolve(false);
                } catch (DIDException e) {
                    e.printStackTrace();
                    return null;
                }
                return didDocument;
            }

            @Override
            protected void onPostExecute(DIDDocument didDocument) {
                if (didDocument == null) {
                    Log.d(LOG_TAG, "Empty did document. No additional contact info fond on DID sidechain");
                    listener.onDIDInfo(null, null);
                }
                else {
                    Log.d(LOG_TAG, "Contact Did document found. Trying to extract name and avatar");
                    String name = null, avatarHash = null;

                    // Try to find a name credential
                    VerifiableCredential nameCredential = didDocument.getCredential("name");
                    if (nameCredential != null) {
                        Log.d(LOG_TAG, "Found a name credential");
                        name = nameCredential.getSubject().getPropertyAsString("name");
                    }
                    else {
                        Log.d(LOG_TAG, "No name credential found");
                    }

                    // TODO: try to find the avatar hash

                    listener.onDIDInfo(name, avatarHash);
                }
            }
        }.execute();
    }

    private interface OnChainDIDInfoReceived {
        void onDIDInfo(String name, String avatarIPFSHash);
    }
}
