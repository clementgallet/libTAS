---
layout: page
title: News
permalink: /news/
---

{% for post in site.posts %}
  <h4><a href="{{ post.url | relative_url }}">{{ post.title }}</a></h4>
  
  {{ post.excerpt }}

{% endfor %}
