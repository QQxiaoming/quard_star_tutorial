(: Name: extexpr-7 :)
(: Description: An extension expression for which the pragma is ignored and default expression is a quantified expression ("every" operator). :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(# ns1:you-do-not-know-me-as-index #)
     {fn:string(every $x in (1,2,3) satisfies $x < 4)}
