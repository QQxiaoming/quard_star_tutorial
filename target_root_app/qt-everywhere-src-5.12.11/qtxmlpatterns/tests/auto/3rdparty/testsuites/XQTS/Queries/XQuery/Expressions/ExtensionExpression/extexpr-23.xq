(: Name: extexpr-23 :)
(: Description: An extension expression for which the pragma is ignored and default expression is a multiplication operation. :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(# ns1:you-do-not-know-me-as-index #)
{10 * 2}