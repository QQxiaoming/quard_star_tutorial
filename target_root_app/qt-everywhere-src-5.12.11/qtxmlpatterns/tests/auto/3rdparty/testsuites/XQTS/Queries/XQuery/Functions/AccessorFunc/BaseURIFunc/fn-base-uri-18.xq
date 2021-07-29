(: Name: fn-base-uri-18 :)
(: Description: Evaluation of base-uri function with argument set to a directly constructed element node (via FLOWR expr). :)
(: Use the declare base-uri :)

declare base-uri "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>With some contexnt</anElement>
return
   fn:string(fn:base-uri($var))