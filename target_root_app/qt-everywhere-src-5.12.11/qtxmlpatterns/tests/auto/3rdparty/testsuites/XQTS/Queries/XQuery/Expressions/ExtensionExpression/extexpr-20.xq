(: Name: extexpr-20 :)
(: Description: An extension expression for which the pragma is ignored and default expression uses the fn:count function. :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(# ns1:you-do-not-know-me-as-index #)
{fn:count((1,2,3))}