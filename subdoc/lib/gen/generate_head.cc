// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "subdoc/lib/gen/generate_head.h"

namespace subdoc::gen {

void generate_head(HtmlWriter& html, std::string_view title,
                   std::string_view description,
                   const Options& options) noexcept {
  {
    auto head = html.open_head();
    {
      auto meta = head.open_meta();
      meta.add_name("generator");
      meta.add_content("subdoc");
    }
    {
      auto meta = head.open_meta();
      meta.add_name("viewport");
      meta.add_content("width=device-width, initial-scale=1");
    }
    {
      auto meta = head.open_meta();
      meta.add_property("og:type");
      meta.add_content("website");
    }
    {
      auto meta = head.open_meta();
      meta.add_property("og:site_name");
      meta.add_content(options.project_name);
    }

    auto page_title = std::string(title);
    if (!page_title.empty()) {
      page_title += " - ";
    }
    page_title += options.project_name;

    {
      auto title_tag = head.open_title();
      title_tag.write_text(page_title);
    }
    {
      auto meta = head.open_meta();
      meta.add_property("og:title");
      meta.add_content(page_title);
    }
    {
      auto meta = head.open_meta();
      meta.add_name("description");
      meta.add_content(description);
    }
    {
      auto meta = head.open_meta();
      meta.add_property("og:description");
      meta.add_content(description);
    }

    // TODO: Package this into the Subdoc output to avoid an external
    // dependency.
    {
      auto script = head.open_script(HtmlWriter::SingleLine);
      script.add_src("https://unpkg.com/lunr/lunr.js");
    }

    // Script for showing search results.
    {
      auto script = head.open_script(HtmlWriter::SingleLine);
      script.add_src("./search_db.js");
    }
    {
      auto script = head.open_script();
      script.write_html(R"#(
        // Delayed loading of whatever was in the search box.
        var searchDelayLoad;

        // The search box's dynamic behaviour.
        document.addEventListener("keyup", e => {
          if (e.key === 's') {
            document.querySelector('.search-input').focus();
          }
          if (e.key === 'Escape') {
            document.querySelector('.search-input').blur();
            navigateToSearch(null);
            e.preventDefault();
          }
        });
        function navigateToSearch(query) {
          window.clearTimeout(searchDelayLoad);
          searchDelayLoad = null;

          let without_search =
              window.location.origin + window.location.pathname;
          if (query) {
            window.history.replaceState(null, "",
              without_search + "?" + `search=${query}`);
          } else {
            window.history.replaceState(null, "", without_search);
          }
          maybeShowSearchResults();
        }
        addEventListener("load", event => {
          document.querySelector(".search-input").oninput = (e) => {
            window.clearTimeout(searchDelayLoad);
            searchDelayLoad = window.setTimeout(() => {
              navigateToSearch(e.target.value);
            }, 1000);
          };
          document.querySelector(".search-input").onkeypress = (e) => {
            if (e.key == "Enter") {
              navigateToSearch(e.target.value);
              e.preventDefault();
            }
          };
          var searchPlaceholder;
          document.querySelector(".search-input").onfocus = (e) => {
            searchPlaceholder = e.target.placeholder;
            e.target.placeholder = "Type your search here.";
            navigateToSearch(e.target.value);
          };
          document.querySelector(".search-input").onblur = (e) => {
            e.target.placeholder = searchPlaceholder;
            searchPlaceholder = null;
          };
        });

        // Show or hide any DOM element.
        function showHide(selector, show) {
          if (show)
            document.querySelector(selector).classList.remove("hidden");
          else
            document.querySelector(selector).classList.add("hidden");
        }

        function searchQuery() {
          const params = new Proxy(
            new URLSearchParams(window.location.search), {
              get: (searchParams, prop) => searchParams.get(prop),
            }
          );
          return params.search;
        }

        // Showing search results.
        async function populateSearchResults(loaded) {
          const search_db = loaded.search_db;
          const idx = loaded.idx;

          // lunrjs treats `:` specially and `::` breaks the query syntax, so
          // just split into two words.
          const query = searchQuery().split("::").join(" ");
          let content = '';
          try {
            const results = idx.search(query);
            for (r of results) {
              const item = search_db[r.ref];

              const type = item.type;
              const url = item.url;
              const name = item.name;
              const full_name = item.full_name;
              const summmary = item.summary ? item.summary : "";

              content += `\
                <a class="search-results-link" href="${url}">
                  <span class="search-results-type">${type}</span>\
                  <span class="search-results-name">${full_name}</span>\
                  <span class="search-results-summary">${summmary}</span>\
                </a>\
                `
            }
          } catch (err) {
            content +=
                `<div class="search-error">Search error: ${err.message}</div>`;
          }

          let content_elem = document.querySelector(".search-results-content");
          content_elem.innerHTML = content;

          let header_elem = document.querySelector(".search-results-header");
          header_elem.innerText = "Search results";
        }

        var cache_idx;

        // Searching via https://lunrjs.com.
        //
        // Load the JSON search database, which will be turned into a search
        // index. Returns an object with two fields:
        // - search_db: the contents of the search.json file.
        // - idx: the lunr search index.
        //   Documented at https://lunrjs.com/docs/lunr.Index.html.
        async function loadSearchIndex() {
          // This is not widely supported yet (not on Safari), so instead we
          // turned the json file into a js file that sets a global variable. :|
          //async function load_search_db() {
          //  let x = await import('./search.json', {
          //    with: {type: 'json'}
          //  });
          //  return x.default;
          //}

          async function load_idx(search_db) {
            let idx = lunr(function () {
              this.ref('index');
              this.field('name', {
                'boost': 2
              });
              this.field('full_name', {
                'boost': 2
              });
              this.field('split_name', {
                'boost': 0.5
              });
              this.field('summary', {
                'boost': 1
              });
              this.field('full_description', {
                'boost': 0.75
              });

              // No stemming?
              // this.pipeline = new lunr.Pipeline();

              this.use(builder => {
                function splitColons(token) {
                  return token.toString().split("::").map(str => {
                    return token.clone().update(() => { return str })
                  })
                }
                lunr.Pipeline.registerFunction(splitColons, 'splitColons')
                builder.searchPipeline.before(lunr.stemmer, splitColons)
              });

              search_db.forEach(item => {
                this.add(item, {
                  'boost': item.weight
                })
              }, this);
            });
            let out = {};
            out.search_db = search_db;
            out.idx = idx;
            return out;
          };

          if (!cache_idx) {
            cache_idx = await load_idx(g_search_db);
          }
          return cache_idx;
        }

        // If there's a search query, hide the other content and asynchronously
        // show the search results. Otherwise, hide search content and show the
        // rest immediately.
        function maybeShowSearchResults() {
          const query = searchQuery();
          if (query) {
            showHide(".main-content", false);

            let input = document.querySelector(".search-input");
            input.value = query;

            let header_elem = document.querySelector(".search-results-header");
            header_elem.innerText = "Loading search results...";

            let content_ele = document.querySelector(".search-results-content");
            content_ele.innerText = "";

            loadSearchIndex().then(populateSearchResults)
          } else {
            showHide(".main-content", true);

            let header_elem = document.querySelector(".search-results-header");
            header_elem.innerText = "";

            let content_ele = document.querySelector(".search-results-content");
            content_ele.innerText = "";
          }
        }

        )#");
    }

    for (const std::string& path : options.stylesheets) {
      auto stylesheet_link = head.open_link();
      stylesheet_link.add_rel("stylesheet");
      stylesheet_link.add_href(path);
    }
    for (const auto& [i, favicon] : options.favicons.iter().enumerate()) {
      auto favicon_link = head.open_link();
      if (i == 0u)
        favicon_link.add_rel("icon");
      else
        favicon_link.add_rel("alternate icon");
      favicon_link.add_type(favicon.mime);
      favicon_link.add_href(favicon.path);
    }
    if (Option<const FavIcon&> icon = options.favicons.first();
        icon.is_some()) {
      auto meta = head.open_meta();
      meta.add_property("og:image");
      meta.add_content(icon.as_value().path);
    }
  }
  html.write_empty_line();
}

}  // namespace subdoc::gen
