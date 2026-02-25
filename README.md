# cdn-cache

1)
Vyhledává podle klíče, který byl vytvořen hashováním proměnných requestu, např. $scheme$proxy_host$request_uri. Nejdříve se vezmou tyto proměnné a ve funkci ngx_http_proxy_create_key se naplní pole keys (typ ngx_array_t) ve struktuře r->cache (typ ngx_http_cache_t, ukazatel v ngx_http_request_s). Pole keys se pak použije ve funkci ngx_http_file_cache_create_key, kde se z něj vytvoří hash crc32 a pomocí MD5 klíč, který se pak používá při vyhledávání (viz níže). Crc32 se později používá při čtení souboru, jako finální kontrola že nedošlo k md5 kolizi a file tam tak ve skutečnosti není.
Z hlediska paměťového umístění: r->cache, r->cache->keys a r->cache->key jsou per-request data, která po vykonání requestu zaniknou. Naopak keys_zone je sdílený paměťový segment, který po requestu nezaniká (data jsou uložena ve struktuře ngx_http_file_cache_sh_t, více viz níže).

Struktura, která globálně drží informaci o tom, jaké cache položky jsou v konkrétní keys_zone, je ngx_http_file_cache_sh_t. Její konfigurace je uložená v ngx_http_file_cache_t. Jednotlivé klíče/položky jsou ngx_http_file_cache_node_t.
  - Co si v sobě drží ngx_http_file_cache_node_t:
    - ukazatel na uzel ve stromu (ngx_rbtree_node_t), který obsahuje část klíče pro řazení; zbytek klíče je uložen v ngx_http_file_cache_node_t,
    - ukazatel na pozici v obousměrně zřetězeném seznamu (ngx_queue_t).
      - pokud se položka dostane na konec seznamu, je „expired“ a zároveň není používána (count = 0), tak ji vyhodí; jinak ji posune zpět na začátek fronty.
       
2)
Vytvořil jsem si vlastní modul v adresáři /modules podle dokumentace, abych splnil minimální požadavky, a díky tomu jsem ho měl zařazený v modules[] a následně se i vykonával. Protože jde o addon modul a není součástí core, vytvořil jsem k němu config skript.
Vytvořil jsem funkci ngx_x_cache_key_header_filter, která si vezme už hotový klíč z requestu (je volaná v post configuration, takže klíč už existuje) -> ten převede na hexadecimální číslo (32 bytů), udělá pushback na headers_out a pushback jí vrátí prázdný ngx_table_elt_t, do kterého vyplní už klíč a předá informaci, že to je hash.

3)
Pro hledání suffixů a prefixů se používá datová struktura hash trie, pro exact match normální hash tabulka.
Proč tvůrci zvolili hash trie?
  - Šetří místo -> pokud by použili normální trie tak by museli v každém Nodu inicializovat velký array, který by byl často využit jen s pár procent
  - Rychlost -> místo arraye si každý node drží hash tabulku takže každý lookup je O(1)
  

Jak to je implementováno v Nginx?
ngx_hash_combined_t drží:
  - hash -> exact hash tabulku pro přesné názvy
  - wc_head -> wildcard suffix hash trie pro *.example.com
  - wc_tail -> wildcard prefix hash trie pro example.*


time spent cca 7h
