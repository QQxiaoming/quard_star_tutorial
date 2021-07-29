(: Name: fn-namespace-uri-for-prefix-4 :)
(: Description: Evaluation of fn:namespace-uri-for-prefix function,with the first argument (prefix) set to the empty sequence and element is directly constructed. :)
(: Use fn:string. :)

declare default element namespace "http://www.example.com/defaultspace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement xmlns:p1 = "http://www.example.com/examples">some context</anElement>
return
 fn:string(fn:namespace-uri-for-prefix((),$var))