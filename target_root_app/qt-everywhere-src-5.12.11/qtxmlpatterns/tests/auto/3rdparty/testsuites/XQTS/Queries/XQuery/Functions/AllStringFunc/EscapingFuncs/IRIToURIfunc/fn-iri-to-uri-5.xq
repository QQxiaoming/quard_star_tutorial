(: Name: fn-iri-to-uri-5 :)
(: Description: Examines that the fn:iri-to-uri function does not escape the "_" symbol.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:iri-to-uri("example_example")