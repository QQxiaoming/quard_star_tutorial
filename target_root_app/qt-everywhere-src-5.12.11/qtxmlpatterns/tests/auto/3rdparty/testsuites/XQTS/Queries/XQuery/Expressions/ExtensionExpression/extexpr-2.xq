(: Name: extexpr-2 :)
(: Description: An extension expression with no expression :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 (# ns1:you-do-not-know-me-as-index #)
      {}