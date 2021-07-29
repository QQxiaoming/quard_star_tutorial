(: Name: fn-namespace-uri-for-prefix-5 :)
(: Description: Evaluation of fn:namespace-uri-for-prefix function,with the first argument (prefix) set to the prefix of a directly constructed element given as argument. :)
(: Use fn:string. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:namespace-uri-for-prefix("p1",<anElement xmlns:p1 = "http://www.example.com/examples">some context</anElement>))