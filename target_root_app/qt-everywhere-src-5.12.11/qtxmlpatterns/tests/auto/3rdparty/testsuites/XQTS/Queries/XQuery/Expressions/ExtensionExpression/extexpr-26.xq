(: Name: extexpr-26 :)
(: Description: Extension expression for missing space after pragma name. :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(# ns1:you-do-not-know-me-as-index#){fn:count((1,2,3))}