(: Name: extexpr-15 :)
(: Description: An extension expression for which the pragma is ignored and default expression is an "if" true expression :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(# ns1:you-do-not-know-me-as-index #)
     {if(fn:true()) then "passed" else "failed"}
