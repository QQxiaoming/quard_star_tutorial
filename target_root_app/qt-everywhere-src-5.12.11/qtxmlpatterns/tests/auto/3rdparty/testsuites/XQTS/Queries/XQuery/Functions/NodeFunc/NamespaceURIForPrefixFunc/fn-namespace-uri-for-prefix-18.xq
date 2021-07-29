(: Name: fn-namespace-uri-for-prefix-18 :)
(: Description: Evaluation of fn:namespace-uri-for-prefix function for which the element is computed as variable (via let) with no namespace. :)
(: Use fn:count to avoid empty file. :)

declare default element namespace "http://www.example.com/defaultnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := element anElement {"some content"}
return
 fn:count(fn:namespace-uri-for-prefix("p1",$var))