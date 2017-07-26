<?php

function extractSlugFromUrl($url) {
  $exploded = array_filter(explode('/', $url));
  if (strcmp(end($exploded), 'dashboard') == 0) {
    array_pop($exploded);
  }
  return end($exploded);
}
