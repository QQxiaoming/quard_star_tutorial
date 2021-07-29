(: Name: extexpr-6 :)
(: Description: A FLOWR expression that uses the same pragma twice and both times should be ignored. :)

declare namespace ns1 = "http://example.org/someweirdnamespace";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in (# ns1:you-do-not-know-me-as-index #)
     {$input-context1/works/employee[12]/overtime}
return
     (# ns1:you-do-not-know-me-as-index #)
     {$input-context1/works/employee[12]/overtime}