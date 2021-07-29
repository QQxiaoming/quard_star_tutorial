(: Name: fn-base-uri-21 :)
(: Description: Evaluation of base-uri function with argument set to a directly constructed comment node (via FLOWR expr). :)
(: Should ignore declared base uri property. :)

declare base-uri "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <!-- A Comment -->
return
   fn:count(fn:base-uri($var))