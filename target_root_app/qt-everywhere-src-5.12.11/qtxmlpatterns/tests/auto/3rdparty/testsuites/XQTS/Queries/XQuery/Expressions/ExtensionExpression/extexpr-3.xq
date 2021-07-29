(: Name: extexpr-3 :)
(: Description: An pragma content containing the "#" symbol, which is ignored in an extension expression :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 (# ns1:you-do-not-know-me-as-index "ABC#" #)
      {$input-context1/works/employee[12]/overtime}