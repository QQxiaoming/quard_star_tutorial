(: Name: extexpr-5 :)
(: Description: An extension expression that contains more than one pragma, both of wihc are ignored :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 (# ns1:you-do-not-know-me-as-index  #)
 (# ns1:you-should-not-know-me-either #)
      {$input-context1/works/employee[12]/overtime}