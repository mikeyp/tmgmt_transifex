## Setting up the dev environment with Vagrant

### Starting from here

You would need virtualbox and vagrant so go to their sites and retrieve the latest
dmg.

### installation
```
  cd ..
  cd ..
  git clone https://github.com/geerlingguy/drupal-vm tx-drupal-vm
  cd tx-drupal-vm
  cp ./../tmgmt_transifex/vagrant/*.yml ./
  vagrant up
```

Provide your master password in order to automatically set /etc/hosts, and then
you can login to your Drupal 7 installation from drupalvm.dev (admin/admin).
Furthermore the tmgmt_transifex directory is mounted inside the vm at Drupal's module directory.

### Activate required modules

```
  vagrant ssh
  drush en views_bulk_operations ctools entity entity_translation rules title tmgmt tmgmt_ui tmgmt_node tmgmt_node_ui views --y
```

### Configure tmgmt and Transifex connector

1. Go to admin/modules find Transifex Connector and activate
2. Obtain an api key from Transifex https://www.transifex.com/user/settings/api/
3. Create a new project in Transifex
4. Configure Transifex Connector by providing the apikey and the target project slug (admin/config/regional/tmgmt_translator/manage/transifex)
5. Add target languages to your Drupal instance (admin/config/regional/language)
6. Activate localization on selected content types.
  * Go to admin/structure/types/manage/<content-type>
  * Click on the publishing options tab
  * Select  Enabled, with translation or Enabled, with field translation

### Push content to TX

7. Hit the Translation submenu, and go to sources
  * Add the nodes you want translated to your cart
  * Then go to cart select target language and then create a job
  * Submit the job, you should get back a report on the uploaded strings.
