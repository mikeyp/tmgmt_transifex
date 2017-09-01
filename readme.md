# Localizing your Drupal site with Transifex
Transifex’s Drupal integration lets you translate your Drupal site using Transifex. It works as an add on to [Translation Management Tool (TMGMT)](https://www.drupal.org/project/tmgmt), the de-facto Drupal module for localization backed by Microsoft, Acquia, and others. For now, only Drupal 7 is supported.

This guide will walk you through the process of setting up TMGMT, configuring the [Transifex connector](https://www.drupal.org/project/tmgmt_transifex), sending content from Drupal to Transifex, and getting translations back in an automated way.

## Development

For local development you can use the provided vagrant image.
For deploying drupal to a server you can use the scripts found in ansible.

## Releasing

In order to release a new version of the project to drupal.org and make it available to
Drupal users do the following:

* From inside this repo add a drupal origin like so:
```
  git remote add drupal transifex@git.drupal.org:project/tmgmt_transifex.git
```

* Checkout the 7.x-1.x branch

* Push your local branch to drupal.org
```
  git push drupal 7.x-1.x
```
* Create a tag and push it to drupal.org
```
  git tag 7.x-0.9
  git push drupal tag 7.x-0.9
```
* Go to https://www.drupal.org/project/tmgmt_transifex, login and click edit
* Go to the releases tab and click on ```Add new release``` and publish your new version


## Installation and configuration
### Installing TMGMT

Before you can install the Transifex connector, you will need to install the TMGMT module along with its dependencies. Here’s how:

![](https://d2mxuefqeaa7sj.cloudfront.net/s_22E80DB6982BD9F76720921769EA440F9AA7B35EE44FF5F7AFC9FAADA289B080_1503322795737_Screen+Shot+2017-08-21+at+16.39.26.png)



1. Assuming you have [drush](http://www.drush.org/en/master/) installed from inside your site’s folder, issue the following command:

```
    drush dl tmgmt entity views ctools rules views_bulk_operations entity_translation --y
```



2. After you've pulled the modules, enable them all by requesting activation:
```
    drush en tmgmt_ui tmgmt_entity_ui tmgmt_node_ui --y
```

If successful, you should see a translate submenu appear in the top admin bar.


### Configuring languages for your Drupal site

Next, you can add the languages that you want your site translated to:


1. From the main navigation, head to **Configuration**. Then under **REGIONAL AND LANGUAGE**, click on **Languages**.
2. Click on **+Add language**.
3. Select a language from the list and then click **Add language**.
4. Repeat Steps 2 and 3 to add all the languages you wish to translate your Drupal site to.

If a language you want isn’t in the list, you can add it through the **Custom Language** section.

### Configuring Translation sources

After installing TMGMT, you need to denote certain content types that will be translatable. To do that go to **Structure > Content Types** select the one you want, click on edit and then from the publishing options tab select ‘Enabled, with field translation’. **Take note,** that existing content wont take the latter configuration, it is only applied to new content. If all went well you can see the translatable content at **Translation > Sources > Content.**

![](https://d2mxuefqeaa7sj.cloudfront.net/s_22E80DB6982BD9F76720921769EA440F9AA7B35EE44FF5F7AFC9FAADA289B080_1503660918843_Screen+Shot+2017-08-25+at+14.28.39.png)


You can configure further the translation of the content type by going to **admin/config/regional/entity_translation.** Its advised that you set the default language to your source language and not language_neutral and also make sure
that all fields of the content type are marked for translation. From here you can also configure Language fallback.

![](https://d2mxuefqeaa7sj.cloudfront.net/s_22E80DB6982BD9F76720921769EA440F9AA7B35EE44FF5F7AFC9FAADA289B080_1503662314222_Screen+Shot+2017-08-25+at+14.55.04.png)


Since we are using field translation we also need to denote which fields will be translatable, to do that go to **Structure > Content Types** > your content type, click on edit fields, and from the popup enable localization for that field.

### (Optional) Enable localizable node titles

By default Drupal does not support multiple values for the title field, as a workaround you would need to install the Title module
```
    drush en title
```

And then go to Content Types, select the one you want to have localizable titles, go to fields and hit the replace button next to the title field.

### Installing Transifex Connector

In order to import content to Transifex you need to install the Transifex Connector. Do that with:
```
    drush dl tmgmt_transifex
    drush en tmgmt_transifex
```
From **Configuration > Translation Management Translators** select Transifex Translator and fill the configuration form with your credentials.

![](https://d2mxuefqeaa7sj.cloudfront.net/s_22E80DB6982BD9F76720921769EA440F9AA7B35EE44FF5F7AFC9FAADA289B080_1503933600812_Screen+Shot+2017-08-28+at+18.18.55.png)


At this stage you have two choices regarding your localization workflow. You can either consider translations coming from Transifex as ready to be imported either if they are 100% translated or 100% reviewed. So if you want also a review step to your process enable the Pull resources settings so translations get imported only when they are 100% reviewed.


To create a file-based project for use with the drupal integration go to → https://www.transifex.com/your-organization/add/. If you also want to add a webhook, go to your project’s dashboard select Settings > Webhooks and set your URL target to be http://<your drupal instance>/tmgmt_transifex/webhook and also set event to **Any Event.**    


![](https://d2mxuefqeaa7sj.cloudfront.net/s_22E80DB6982BD9F76720921769EA440F9AA7B35EE44FF5F7AFC9FAADA289B080_1503326607574_Screen+Shot+2017-08-21+at+17.36.25.png)


You will also need an API token for Transifex that can be collected from https://www.transifex.com/user/settings/.

## Usage

### Importing content  

Open the Translate submenu at the top of your admin bar and select the Sources tab, from there click on content and select the content that you want to make available for translation.

![](https://d2mxuefqeaa7sj.cloudfront.net/s_22E80DB6982BD9F76720921769EA440F9AA7B35EE44FF5F7AFC9FAADA289B080_1503504364178_Screen+Shot+2017-08-23+at+19.05.38.png)


From there you can either add them to the cart or request their translation. At the request translation menu you can review the contents of your order, set the jobs label and set the target language. You will need one translation job for each target language.


![](https://d2mxuefqeaa7sj.cloudfront.net/s_22E80DB6982BD9F76720921769EA440F9AA7B35EE44FF5F7AFC9FAADA289B080_1503504515551_Screen+Shot+2017-08-23+at+19.07.33.png)


By clicking submit to translator, the plugin will create matching resources for each node at Transifex.  You can see your strings by going to your Transifex dashboard.

### Pulling translations

There are two ways to retrieve translated content back from Transifex.


- **Webhooks:** In that case when a resource **review** in Transifex reaches 100% a webhook is submitted targeted towards your Drupal installation.  When the webhook is received by the Transifex Connector it will query Transifex in order to retrieve the translations and uses TMGMT in order to apply the translations for the site.  In order to set up webhooks you first need to configure them in Transifex’s side by going to <org>/<project>/settings/webhooks/ and adding a webhook that points to:  

```
     http://<your-drupal-instance>/tmgmt_transifex/webhook
     ```


![](https://d2mxuefqeaa7sj.cloudfront.net/s_22E80DB6982BD9F76720921769EA440F9AA7B35EE44FF5F7AFC9FAADA289B080_1503663399319_Screen+Shot+2017-08-25+at+15.16.12.png)

  *The translations are auto-accepted so whenever a resource reaches 100% in Transifex its automatically updated in Drupal.*


- **Manual**: By using the Translate submenu, go to the jobs tab and click on the job you want to check for translations.
![](https://d2mxuefqeaa7sj.cloudfront.net/s_22E80DB6982BD9F76720921769EA440F9AA7B35EE44FF5F7AFC9FAADA289B080_1503663438789_Screen+Shot+2017-08-25+at+15.16.18.png)


The module will first check if the resource has reached 100% translation and if its true then its going to import translations inside Drupal.
