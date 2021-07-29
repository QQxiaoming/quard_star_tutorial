(: Name: namespaceDecl-7 :)
(: Description: Evaluation usage of namespace declaration, different prefix bounded to same namespace uri and use of same local name (example 2 of this section:)
(: from the query specs. :)

declare namespace xx = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $i := <foo:bar xmlns:foo = "http://example.org">
              <foo:bing> Lentils </foo:bing>
          </foo:bar>
return $i/xx:bing

