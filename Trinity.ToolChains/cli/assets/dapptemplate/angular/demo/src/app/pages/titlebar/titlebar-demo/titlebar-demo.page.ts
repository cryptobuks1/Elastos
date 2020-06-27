import { Component, OnInit } from '@angular/core';
import { NavParams } from '@ionic/angular';
import { TitlebarService } from 'src/app/services/titlebar.service';

declare let titleBarManager: TitleBarPlugin.TitleBarManager;

@Component({
  selector: 'app-titlebar-demo',
  templateUrl: './titlebar-demo.page.html',
  styleUrls: ['./titlebar-demo.page.scss'],
})
export class TitlebarDemoPage implements OnInit {

  public manager: any;
  public titleToggled: boolean = false;
  public colorToggled: boolean = false;

  constructor(
    private navParams: NavParams,
    private titlebarService: TitlebarService
  ) { }

  ngOnInit() {
    this.manager = this.navParams.get('manager');
    console.log('Titlebar Example', this.manager);

    if(this.manager.type === 'Icons') {
      this.titlebarService.setIcon();
    }
    if(this.manager.type === 'Items') {
      this.titlebarService.setMenuItems();
      titleBarManager.addOnItemClickedListener(this.titlebarService.onItemClickedListener = (menuIcon) => {
        if (menuIcon.key === "help") {
          console.log("Help item clicked");
          this.titlebarService.alertTitlebarItemClicked(
            'You clicked a Titlebar item!', 
            'This action won\'t do anything but demonstrate the Titlebar item.'
          );
        }
      });
    }
  }

  ionViewWillEnter() {
    titleBarManager.setTitle('Titlebar Demo');
    this.titlebarService.setTitleBarBackKeyShown(true);
  }

  ionViewWillLeave() {
    this.titlebarService.setTitleBarBackKeyShown(false);
  }

  toggleTitle() {
    if(this.titleToggled) {
      this.titleToggled = false;
      titleBarManager.setTitle('Titlebar Demo');
    } else {
      this.titleToggled = true;
      titleBarManager.setTitle('Titlebar Changed!');
    }
  }

  toggleColor() {
    if(this.colorToggled) {
      this.colorToggled = false;
      titleBarManager.setBackgroundColor("#181d20");
      titleBarManager.setForegroundMode(TitleBarPlugin.TitleBarForegroundMode.LIGHT);
    } else {
      this.colorToggled = true;
      titleBarManager.setBackgroundColor("#ffffff");
      titleBarManager.setForegroundMode(TitleBarPlugin.TitleBarForegroundMode.DARK);
    }
  }
}