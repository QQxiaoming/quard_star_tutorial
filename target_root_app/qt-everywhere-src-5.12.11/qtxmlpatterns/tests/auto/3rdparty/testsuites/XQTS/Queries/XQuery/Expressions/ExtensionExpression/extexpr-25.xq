(: Name: extexpr-25 :)
(: Description: An extension expression for which the pragma is ignored and default expression is a division operation (idiv operator). :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(# ns1:you-do-not-know-me-as-index #)
{10 idiv 2}