(: Name: extexpr-16 :)
(: Description: An extension expression for which the pragma is ignored and default expression is an "if" false expression (returns "else" part of expression) :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(# ns1:you-do-not-know-me-as-index #)
     {if(fn:false()) then "failed" else "passed"}
