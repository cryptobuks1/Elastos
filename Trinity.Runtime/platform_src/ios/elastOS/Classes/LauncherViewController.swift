 /*
  * Copyright (c) 2018 Elastos Foundation
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  */

 import Foundation

 @objc(LauncherViewController)
 class LauncherViewController : TrinityViewController {

    override func loadSettings() {
        super.loadSettings();

        self.pluginObjects["CDVIntentAndNavigationFilter"] = self.whitelistFilter;

        if (AppViewController.originalPluginsMap.isEmpty) {
            for (key, value) in pluginsMap {
                let name = key as! String;
                let className = value as! String;
                AppViewController.originalPluginsMap[name] = className;
            }
        }

        if (AppViewController.originalStartupPluginNames.isEmpty) {
            for item in self.startupPluginNames {
                let name = item as! String;
                AppViewController.originalStartupPluginNames.append(name);
            }
        }

        AppViewController.originalSettings = self.settings;
        self.settings.setValue(getCustomHostname(did, packageId), forKey: "hostname");

        if(self.wwwFolderName == nil){
            self.wwwFolderName = "www";
        }
        self.startPage = AppManager.getShareInstance().getStartPath(self.appInfo!);
    }

 }