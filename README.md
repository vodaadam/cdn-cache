# cdn-cache

1)
Vyhledává podle klíče, který byl vytvořen hashováním proměnných requestu, např. $scheme$proxy_host$request_uri. Nejdříve se vezmou tyto proměnné a ve funkci ngx_http_proxy_create_key se naplní pole keys (typ ngx_array_t) ve struktuře r->cache (typ ngx_http_cache_t, ukazatel v ngx_http_request_s). Pole keys se pak použije ve funkci ngx_http_file_cache_create_key, kde se z něj vytvoří hash crc32 a pomocí MD5 klíč, který se pak používá při vyhledávání (viz níže).
Z hlediska paměťového umístění: r->cache, r->cache->keys a r->cache->key jsou per-request data, která po vykonání requestu zaniknou. Naopak keys_zone je sdílený paměťový segment, který po requestu nezaniká (data jsou uložena ve struktuře ngx_http_file_cache_sh_t, více viz níže).

Struktura, která globálně (ve sdílené paměti) drží informaci o tom, jaké cache položky jsou v konkrétní keys_zone, je ngx_http_file_cache_sh_t. Její konfigurace je uložená v ngx_http_file_cache_t. Jednotlivé klíče/položky jsou ngx_http_file_cache_node_t.
  - Co si v sobě drží ngx_http_file_cache_node_t:
    - ukazatel na uzel ve stromu (ngx_rbtree_node_t), který obsahuje část klíče pro řazení; zbytek klíče je uložen v ngx_http_file_cache_node_t,
    - ukazatel na pozici v obousměrně zřetězeném seznamu (ngx_queue_t).
      - pokud se položka dostane na konec seznamu, je „expired“ a zároveň není používána (count = 0), tak ji vyhodí; jinak ji posune zpět na začátek fronty.
       
