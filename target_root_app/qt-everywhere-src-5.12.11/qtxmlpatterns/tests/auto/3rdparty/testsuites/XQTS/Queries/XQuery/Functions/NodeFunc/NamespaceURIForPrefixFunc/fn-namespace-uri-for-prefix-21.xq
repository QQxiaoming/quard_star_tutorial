(: Name: fn-namespace-uri-for-prefix-21 :)
(: Description: Evaluation of fn:namespace-uri-for-prefix function for which the element is directly constructed and have multiple namespaces. :)
(: Use fn:string. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:namespace-uri-for-prefix("p1",<anElement xmlns:p1="http://www.example.com/examples" xmlns:p2="http://www.someotherns.com/namespace">some content"</anElement>))