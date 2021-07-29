(: Name: extexpr-1 :)
(: Description: A simple call to an extension expression, that should not be recognized (and thus ignored) :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 (# ns1:you-do-not-know-me-as-index #)
      { $input-context1/works/employee[12]/overtime }