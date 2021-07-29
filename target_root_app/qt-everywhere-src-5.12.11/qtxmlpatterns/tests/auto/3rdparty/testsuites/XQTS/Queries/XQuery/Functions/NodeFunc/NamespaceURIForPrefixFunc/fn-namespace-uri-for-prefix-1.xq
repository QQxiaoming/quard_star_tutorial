(: Name: fn-namespace-uri-for-prefix-1 :)
(: Description: Evaluation of fn:namespace-uri-for-prefix function, for which the element is defined and namespace uri is set. Use a direct element. :)


(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement xmlns:p1 = "http://www.example.com/examples">some context</anElement>
return
  string(fn:namespace-uri-for-prefix("p1",$var))