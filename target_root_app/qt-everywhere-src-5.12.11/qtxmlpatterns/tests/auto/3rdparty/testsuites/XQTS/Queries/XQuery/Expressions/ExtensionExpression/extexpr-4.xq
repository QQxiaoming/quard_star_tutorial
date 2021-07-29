(: Name: extexpr-4 :)
(: Description: An extension expression, for which its QName can not be resolved to a namespace URI. :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 (# ns2:you-do-not-know-me-as-index  #)
      {$input-context1/works/employee[12]/overtime}