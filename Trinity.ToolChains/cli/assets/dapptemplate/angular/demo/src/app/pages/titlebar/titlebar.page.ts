import { Component, OnInit } from '@angular/core';
import { TitlebarService } from 'src/app/services/titlebar.service';

declare let titleBarManager: TitleBarPlugin.TitleBarManager;

@Component({
  selector: 'app-titlebar',
  templateUrl: './titlebar.page.html',
  styleUrls: ['./titlebar.page.scss'],
})
export class TitlebarPage implements OnInit {

  constructor(
    public titlebarService: TitlebarService
  ) { }

  ngOnInit() {
  }

  ionViewWillEnter() {
    titleBarManager.setTitle("Titlebar Demo");
    titleBarManager.setBackgroundColor("#222428");
    titleBarManager.setForegroundMode(TitleBarPlugin.TitleBarForegroundMode.LIGHT);
    this.titlebarService.setTitleBarBackKeyShown(true);
  }

  ionViewWillLeave() {
    this.titlebarService.setTitleBarBackKeyShown(false);
  }

}


