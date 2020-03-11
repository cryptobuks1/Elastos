package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.did.DIDDocument;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;

import butterknife.BindView;
import butterknife.OnClick;

public class DidDetailFragment extends BaseFragment {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_didname)
    TextView tvDidname;
    @BindView(R.id.tv_didpk)
    TextView tvDidpk;
    @BindView(R.id.tv_did)
    TextView tvDid;
    @BindView(R.id.tv_validdate)
    TextView tvValiddate;
    @BindView(R.id.tv_edit)
    TextView tvEdit;
    Wallet wallet;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_detail;
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
        //  did.getMethodSpecificId()//"did:ela:xxxx"去除did:ela
        putData(getMyDID().getDIDDocument());

    }

    private void putData(DIDDocument doc) {
        tvDidname.setText(getMyDID().getName(doc));
        tvDid.setText(getMyDID().getDidString());
        tvDidpk.setText(getMyDID().getDidPublicKey(doc));
        tvValiddate.setText(getString(R.string.to) + DateUtil.timeNYR(getMyDID().getExpires(doc), getContext()));

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText("DID");
        // ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.del_icon);
    }

    @OnClick({R.id.tv_edit, R.id.tv_credentialinfo, R.id.tv_did, R.id.tv_didpk, R.id.iv_title_right})
    public void onViewClicked(View view) {
        Bundle bundle;
        switch (view.getId()) {
            case R.id.tv_did:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvDid.getText().toString());
                break;
            case R.id.tv_didpk:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvDidpk.getText().toString());
                break;
            case R.id.tv_edit:
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                start(EditDIDFragment.class, bundle);
                break;
            case R.id.tv_credentialinfo:
                //凭证信息
                bundle = new Bundle();
                start(CredentialFragment.class, bundle);
                break;
            case R.id.iv_title_right:
                //  new CRSignUpPresenter().getFee(didInfo.getWalletId(), MyWallet.IDChain, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", DidDetailFragment.this);
                new DialogUtil().showWarmPrompt1(getBaseActivity(), getString(R.string.abandondidornot), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        /*try {
                           // didStore.deleteDid(did);//删除钱包 没有wallt settore
                        } catch (DIDStoreException e) {
                            e.printStackTrace();
                        }*/
                        // didStore.deactivateDid() todo x
                    }
                });
                break;
        }
    }


}
