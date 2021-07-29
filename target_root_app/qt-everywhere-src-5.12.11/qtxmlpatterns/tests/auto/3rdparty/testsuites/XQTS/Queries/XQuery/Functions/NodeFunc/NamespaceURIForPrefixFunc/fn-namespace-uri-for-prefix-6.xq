(: Name: fn-namespace-uri-for-prefix-6 :)
(: Description: Evaluation of fn:namespace-uri-for-prefix function,with the first argument (prefix) set to a non existent prefix of a directly constructed element given as argument. :)
(: Use fn:count to avaoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:count(fn:namespace-uri-for-prefix("p2",<anElement xmlns:p1 = "http://www.example.com/examples">some context</anElement>))