#include <ngx_config.h>
#include <ngx_http.h>
#include <ngx_core.h>

static ngx_http_output_header_filter_pt ngx_http_next_header_filter;

static ngx_int_t
ngx_http_x_cache_key_header_filter(ngx_http_request_t *r)
{
//    if (r->headers_out.status != NGX_HTTP_OK) {
//        return ngx_http_next_header_filter(r);
//    }
#if (NGX_HTTP_CACHE)
    if (r->cache != NULL) {

        // 2*NGX_HTTP_CACHE_KEY_LEN because there two hexa symbols in byte
        u_char *hex = ngx_pnalloc(r->pool, NGX_HTTP_CACHE_KEY_LEN * 2);
        if (hex == NULL) return NGX_ERROR;

        ngx_hex_dump(hex, r->cache->key, NGX_HTTP_CACHE_KEY_LEN);

        ngx_table_elt_t *h = ngx_list_push(&r->headers_out.headers);
        if (h == NULL) return NGX_ERROR;

        h->hash = 1;
        ngx_str_set(&h->key, "X-Cache-Key");
        h->value.data = hex;
        h->value.len  = NGX_HTTP_CACHE_KEY_LEN * 2;
    }
#endif

    return ngx_http_next_header_filter(r);
}

static ngx_int_t
ngx_http_x_cache_key_filter_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_x_cache_key_header_filter;
    return NGX_OK;
}

static ngx_http_module_t ngx_http_x_cache_key_module_ctx = {
        NULL,
        ngx_http_x_cache_key_filter_init,  // postconfiguration <- because I have filter, if I habe nginx variable I would put in preconfiguration
        NULL, NULL,
        NULL, NULL,
        NULL, NULL
};
ngx_module_t ngx_http_x_cache_key_filter_module = {
        NGX_MODULE_V1,
        &ngx_http_x_cache_key_module_ctx,
        NULL,
        NGX_HTTP_MODULE,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, // <- lifecycle hooks
        NGX_MODULE_V1_PADDING
};


