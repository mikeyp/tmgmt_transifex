## Setting up the dev environment with Vagrant

Starting from here

You would need virtualbox and vagrant so go to their sites and retrieve the latest
dmg.


```
  cd ..
  cd ..
  git clone https://github.com/geerlingguy/drupal-vm tx-drupal-vm
  cd tx-drupal-vm
  cp ./../tmgmt_transifex/vagrant/*.yml ./
  vagrant up
```

Provide your master password in order to automatically set /etc/hosts, and then
you can login to your Drupal 7 installation from drupalvm.dev (admin/admin). Furthermore the tmgmt_transifex directory is mounted inside the vm at Drupal's module directory.

Install required modules

```
  vagrant ssh
  cd drupal
  drush dl ctools entity entity_translation rules tmgmt variable views views_bulk_operations
```
