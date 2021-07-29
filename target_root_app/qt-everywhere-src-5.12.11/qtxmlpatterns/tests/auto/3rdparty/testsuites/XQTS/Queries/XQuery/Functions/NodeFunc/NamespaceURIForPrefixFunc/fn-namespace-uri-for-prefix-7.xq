(: Name: fn-namespace-uri-for-prefix-7 :)
(: Description: Evaluation of fn:namespace-uri-for-prefix function,with the first argument (prefix) uses the xs:string function. Element is directly constructed and given as argument. :)
(: Use fn:string. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:namespace-uri-for-prefix(xs:string("p1"),<anElement xmlns:p1 = "http://www.example.com/examples">some context</anElement>))