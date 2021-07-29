(: Name: fn-namespace-uri-for-prefix-2 :)
(: Description: Evaluation of fn:namespace-uri-for-prefix function, for which the element is defined and namespace uri is not set. Use a direct element. :)
(: Use fn:count to avoid empty sequence. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement xmlns:p1 = "http://www.example.com/examples">some context</anElement>
return
  fn:count(fn:namespace-uri-for-prefix("p2",$var))