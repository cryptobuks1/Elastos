import { Component, OnInit } from '@angular/core';
import { IntentService } from 'src/app/services/intent.service';
import { TitlebarService } from 'src/app/services/titlebar.service';

declare let appManager: AppManagerPlugin.AppManager;
declare let titleBarManager: TitleBarPlugin.TitleBarManager;

@Component({
  selector: 'app-intent',
  templateUrl: './intent.page.html',
  styleUrls: ['./intent.page.scss'],
})
export class IntentPage implements OnInit {

  constructor(
    public intentService: IntentService,
    private titlebarService: TitlebarService
  ) { }

  ngOnInit() {
  }

  ionViewWillEnter() {
    titleBarManager.setTitle("Intent Guide");
    this.titlebarService.setTitleBarBackKeyShown(true);
  }

  ionViewWillLeave() {
    this.titlebarService.setTitleBarBackKeyShown(false);
  }
}
